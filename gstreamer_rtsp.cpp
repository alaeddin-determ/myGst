#include <gst/gst.h>
#include <iostream>
#include <string>

static GstFlowReturn on_new_sample_from_sink(GstElement* sink, gpointer data)
{
    GstSample* sample;
    GstBuffer* buffer;
    GstMapInfo map;

    // Pull sample from appsink
    g_signal_emit_by_name(sink, "pull-sample", &sample);
    if (sample)
    {
        // Get buffer from sample
        buffer = gst_sample_get_buffer(sample);
        if (buffer)
        {
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

static void LinkHlsdemuxWithQtdemux(GstElement* src, GstPad* new_pad, GstElement* data)
{
    GstPad* sink_pad;
    GstElement* tsdemux = (GstElement*)data;

    // Get the sink pad of the decoder
    sink_pad = gst_element_get_static_pad(tsdemux, "sink");

    GstPadLinkReturn ret;
    GstCaps* new_pad_caps = NULL;
    GstStructure* new_pad_struct = NULL;
    const gchar* new_pad_type = NULL;

    std::cout << "Received new pad " << GST_PAD_NAME(new_pad) << " from " << GST_ELEMENT_NAME(src) << std::endl;

    if (gst_pad_is_linked(sink_pad))
    {
        std::cout << "tsdemux is already linked. Ignoring" << std::endl;
        goto exit;
    }

    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_type = gst_structure_get_name(new_pad_struct);
    if (!g_str_has_prefix(new_pad_type, "video/quicktime"))
    {
        std::cout << "It has type " << new_pad_type << "which is not mpegts. Ignoring" << std::endl;
        goto exit;
    }

    ret = gst_pad_link(new_pad, sink_pad);
    if (GST_PAD_LINK_FAILED(ret))
    {
        std::cout << "Type is " << new_pad_type << " but link failed" << std::endl;
    }
    else
    {
        std::cout << "Link succeeded with type " << new_pad_type << std::endl;
    }

exit:
    // Unreference the new pad's caps, if we got them
    if (new_pad_caps != NULL)
        gst_caps_unref(new_pad_caps);

    // Unreference the sink pad
    gst_object_unref(sink_pad);
}

static void LinkQtdemuxWithH265parse(GstElement* src, GstPad* new_pad, GstElement* data)
{
    GstPad* sink_pad;
    GstElement* h265parse = (GstElement*)data;

    // Get the sink pad of the decoder
    sink_pad = gst_element_get_static_pad(h265parse, "sink");

    GstPadLinkReturn ret;
    GstCaps* new_pad_caps = NULL;
    GstStructure* new_pad_struct = NULL;
    const gchar* new_pad_type = NULL;

    std::cout << "Received new pad " << GST_PAD_NAME(new_pad) << " from " << GST_ELEMENT_NAME(src) << std::endl;

    if (gst_pad_is_linked(sink_pad))
    {
        std::cout << "tsdemux is already linked. Ignoring" << std::endl;
        goto exit;
    }

    new_pad_caps = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_type = gst_structure_get_name(new_pad_struct);
    if (!g_str_has_prefix(new_pad_type, "video/x-h265"))
    {
        std::cout << "It has type " << new_pad_type << "which is not mpegts. Ignoring" << std::endl;
        goto exit;
    }

    ret = gst_pad_link(new_pad, sink_pad);
    if (GST_PAD_LINK_FAILED(ret))
    {
        std::cout << "Type is " << new_pad_type << " but link failed" << std::endl;
    }
    else
    {
        std::cout << "Link succeeded with type " << new_pad_type << std::endl;
    }

exit:
    // Unreference the new pad's caps, if we got them
    if (new_pad_caps != NULL)
        gst_caps_unref(new_pad_caps);

    // Unreference the sink pad
    gst_object_unref(sink_pad);
}

int main(int argc, char* argv[])
{
    gst_init(&argc, &argv);
    gst_debug_set_default_threshold(GST_LEVEL_WARNING);

    // Define the pipeline description
    // std::string pipeline_desc = "souphttpsrc location=https://test-streams.mux.dev/x36xhzz/x36xhzz.m3u8 ! hlsdemux connection-speed=4294967 ! tsdemux ! h264parse ! nvh264dec ! appsink name=sink";

    // Define the HLS URL variable
    // std::string hls_url = "https://b-f123ebe5.kinesisvideo.ap-southeast-2.amazonaws.com/hls/v1/getHLSMasterPlaylist.m3u8?SessionToken=CiC5gIU_XZhy1qsMj3ws7d6azywBSFy9vq9sncD8aK8YOxIQ4UWkYc8lxizGPHwRgCzg5hoZyVQ4c9Y1GW-Qyi2hssUDfPxe68ztfxe2oSIgwINwZCzp-z59tYZj6o4GAnuqkRyLGFiSs3yR4JEudOQ~";

    // // Construct the pipeline description string
    // pipeline_desc =
    //     "souphttpsrc location=\"" + hls_url + "\" ! hlsdemux ! qtdemux ! h265parse ! nvh265dec ! appsink name=sink";

    // pipeline_desc =
    //     "souphttpsrc location=\"" + hls_url + "\" ! hlsdemux ! tsdemux ! h264parse ! nvh264dec ! appsink name=sink";

    // Log pipeline description
    // g_print("Creating pipeline with description: %s\n", pipeline_desc.c_str());

    // Log pipeline description
    // g_print("Creating pipeline with description: %s\n", pipeline_desc);

    GstElement* m_souphttpsrc = gst_element_factory_make("souphttpsrc", std::string("souphttpsrc-").c_str());
    GstElement* m_hlsdemux = gst_element_factory_make("hlsdemux", std::string("hlsdemux-").c_str());
    GstElement* m_qtdemux = gst_element_factory_make("qtdemux", std::string("qtdemux-").c_str());
    GstElement* m_h265parse = gst_element_factory_make("h265parse", std::string("h265parse-stream-").c_str());
    GstElement* m_nvh265dec = gst_element_factory_make("nvh265dec", std::string("nvh265dec-").c_str());
    GstElement* m_appsink = gst_element_factory_make("appsink", std::string("appsink-").c_str());

    // Create the pipeline
    GError* error = nullptr;
    GstElement* m_pipeline = gst_pipeline_new(std::string("gst-pipeline-").c_str());

    if (!m_pipeline || !m_souphttpsrc || !m_hlsdemux || !m_qtdemux || !m_h265parse || !m_nvh265dec || !m_appsink)
    {
        g_print("Not all gst elements could be created.");
        return -1;
    }

    // Build the pipeline
    gst_bin_add_many(GST_BIN(m_pipeline), m_souphttpsrc, m_hlsdemux, m_qtdemux, m_h265parse, m_nvh265dec, m_appsink, NULL);

    if (gst_element_link_many(m_souphttpsrc, m_hlsdemux, NULL) != TRUE)
    {
        g_print("Elements PART 1 for streaming could not be linked.");

        gst_object_unref(m_pipeline);
        return -1;
    }

    g_signal_connect(m_hlsdemux, "pad-added", G_CALLBACK(LinkHlsdemuxWithQtdemux), m_qtdemux);
    g_signal_connect(m_qtdemux, "pad-added", G_CALLBACK(LinkQtdemuxWithH265parse), m_h265parse);

    // link streaming elements
    if (gst_element_link_many(m_h265parse, m_nvh265dec, m_appsink, NULL) != TRUE)
    {
        g_print("Elements PART 2 for streaming could not be linked.");

        gst_object_unref(m_pipeline);
        return -1;
    }

    // Modify the url
    std::string url = std::string("https://test-streams.mux.dev/x36xhzz/x36xhzz.m3u8");

    url = "https://b-f123ebe5.kinesisvideo.ap-southeast-2.amazonaws.com/hls/v1/getHLSMasterPlaylist.m3u8?SessionToken=CiA8E12CEZbT4bwqDoVBSMAy-RpF9nH7tDqNwQ2G_E6zuBIQ-urha0c0j-MChQLW3qSC0xoZfh3yzVmSDtjYT4mCuGWZTNobVrjl0_HZfCIgX5oi2ZDPtBgo3vecqzItjvOjzDqTYF5OwWCRprYpk70~";

    g_object_set(m_souphttpsrc, "location", url.c_str(), NULL);

    // Setup appsink
    GstCaps* video_caps;

    // Not sure the format of the video from aws is still P010_10LE, if not, please try NV12
    // gchar *video_caps_text = g_strdup_printf("video/x-raw, format=NV12");
    gchar* video_caps_text = g_strdup_printf("video/x-raw, format=P010_10LE");

    video_caps = gst_caps_from_string(video_caps_text);
    if (!video_caps)
    {
        g_print("gst_caps_from_string fail");
        return -1;
    }
    g_object_set(m_appsink, "caps", video_caps, NULL);

    // Set the appsink to emit signals to get data
    g_object_set(m_appsink, "emit-signals", TRUE, NULL);

    // Connect the new-sample signal to the callback function
    g_signal_connect(m_appsink, "new-sample", G_CALLBACK(on_new_sample_from_sink), NULL);

    // Start playing the pipeline
    g_print("Setting pipeline to playing state\n");
    GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_print("Failed to set pipeline to playing state.\n");
        gst_object_unref(m_pipeline);
        return -1;
    }

    g_print("Pipeline is now playing\n");

    // Wait until error or EOS (End of Stream)
    GstBus* bus = gst_element_get_bus(m_pipeline);
    GstMessage* msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    // Handle the message
    if (msg != nullptr)
    {
        GError* err;
        gchar* debug_info;

        switch (GST_MESSAGE_TYPE(msg))
        {
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
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    gst_object_unref(m_pipeline);

    g_print("Program finished\n");

    return 0;
}