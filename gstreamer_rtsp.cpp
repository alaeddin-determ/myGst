#include <gst/gst.h>
#include <iostream>

static GstFlowReturn on_new_sample_from_sink(GstElement* sink, gpointer data) {
    GstSample* sample;
    GstBuffer* buffer;
    GstMapInfo map;

    // Pull sample from appsink
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (sample) {
        // Get buffer from sample
        buffer = gst_sample_get_buffer(sample);
        if (buffer) {
            // Map buffer for reading
            gst_buffer_map(buffer, &map, GST_MAP_READ);
            // Process the data in map.data and map.size
            std::cout << "Received " << map.size << " bytes of data" << std::endl;
            // Unmap buffer
            gst_buffer_unmap(buffer, &map);
        }
        // Unreference sample
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }

    return GST_FLOW_ERROR;
}

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);
    gst_debug_set_default_threshold(GST_LEVEL_WARNING);

    // Define the pipeline description
    std::string pipeline_desc = "rtspsrc location=rtsp://192.168.8.109:8554/RGBD ! rtph265depay ! h265parse ! nvh265dec ! appsink name=sink";

    // Define the HLS URL variable
    std::string hls_url = "https://b-f123ebe5.kinesisvideo.ap-southeast-2.amazonaws.com/hls/v1/getHLSMasterPlaylist.m3u8?SessionToken=CiC5gIU_XZhy1qsMj3ws7d6azywBSFy9vq9sncD8aK8YOxIQ4UWkYc8lxizGPHwRgCzg5hoZyVQ4c9Y1GW-Qyi2hssUDfPxe68ztfxe2oSIgwINwZCzp-z59tYZj6o4GAnuqkRyLGFiSs3yR4JEudOQ~";

    // Construct the pipeline description string
    pipeline_desc =
        "souphttpsrc location=\"" + hls_url + "\" ! hlsdemux ! qtdemux ! h265parse ! nvh265dec ! appsink name=sink";

    // Log pipeline description
    g_print("Creating pipeline with description: %s\n", pipeline_desc.c_str());


    // Log pipeline description
    g_print("Creating pipeline with description: %s\n", pipeline_desc);

    // Create the pipeline
    GError* error = nullptr;
    GstElement* pipeline = gst_parse_launch(pipeline_desc.c_str(), &error);

    if (!pipeline) {
        g_print("Failed to create pipeline: %s\n", error->message);
        g_error_free(error);
        return -1;
    }

    g_print("Pipeline created successfully\n");

    // Get the appsink element from the pipeline
    GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    if (!sink) {
        g_print("Failed to get appsink element from pipeline\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Set the appsink to emit signals to get data
    g_object_set(sink, "emit-signals", TRUE, NULL);

    // Connect the new-sample signal to the callback function
    g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample_from_sink), NULL);

    // Start playing the pipeline
    g_print("Setting pipeline to playing state\n");
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_print("Failed to set pipeline to playing state.\n");
        gst_object_unref(pipeline);
        return -1;
    }

    g_print("Pipeline is now playing\n");

    // Wait until error or EOS (End of Stream)
    GstBus* bus = gst_element_get_bus(pipeline);
    GstMessage* msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    // Handle the message
    if (msg != nullptr) {
        GError* err;
        gchar* debug_info;

        switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR:
            gst_message_parse_error(msg, &err, &debug_info);
            g_print("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
            g_print("Debugging information: %s\n", (debug_info ? debug_info : "none"));
            g_clear_error(&err);
            g_free(debug_info);
            break;
        case GST_MESSAGE_EOS:
            g_print("End-Of-Stream reached.\n");
            break;
        default:
            // Should not reach here
            g_print("Unexpected message received.\n");
            break;
        }
        gst_message_unref(msg);
    }

    // Free resources
    g_print("Cleaning up and freeing resources\n");
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    g_print("Program finished\n");

    return 0;
}
