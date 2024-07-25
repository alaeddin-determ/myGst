#include <gst/gst.h>
#include <iostream>

int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);

    // Define the pipeline description
    const gchar* pipeline_desc = "rtspsrc location=rtsp://192.168.8.109:8554/RGBD ! rtph265depay ! h265parse ! nvh265dec ! appsink";

    // Log pipeline description
    g_print("Creating pipeline with description: %s\n", pipeline_desc);

    // Create the pipeline
    GError* error = nullptr;
    GstElement* pipeline = gst_parse_launch(pipeline_desc, &error);

    if (!pipeline) {
        g_print("Failed to create pipeline: %s\n", error->message);
        g_error_free(error);
        return -1;
    }

    g_print("Pipeline created successfully\n");

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
