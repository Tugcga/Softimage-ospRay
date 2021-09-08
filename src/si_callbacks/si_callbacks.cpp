#include "si_locker.h"
#include "../utilities/logs.h"
#include "../render_base/render_engine_base.h"
#include "../render_osp/render_engine_osp.h"

RenderEngineOSP g_render;

RenderEngineOSP* render;
RenderEngineOSP* get_render_instance() { return render; }

SICALLBACK OSPRayRenderer_Init(XSI::CRef &in_ctxt)
{
	XSI::Context		ctxt(in_ctxt);
	XSI::Renderer	renderer = ctxt.GetSource();

	XSI::CLongArray	process;
	process.Add(XSI::siRenderProcessRender);
	renderer.PutProcessTypes(process);

	XSI::CString options_name = "OSPRay Renderer Options";
	renderer.AddProperty(XSI::siRenderPropertyOptions, "OSPRay Renderer." + options_name);

	//setup output formats
	renderer.AddOutputImageFormat("Portable Network Graphics (PNG)", "png");
	renderer.AddOutputImageFormatSubType(XSI::siRenderChannelColorType, "RGBA", XSI::siImageBitDepthInteger8);  // use only one type of the data (RGBA or RGB), because we save images by stb_write and it does not simply supports different channels count

	renderer.AddOutputImageFormat(L"Joint Photographic Experts Group (JPG)", "jpg");
	renderer.AddOutputImageFormatSubType(XSI::siRenderChannelColorType, L"RGB", XSI::siImageBitDepthInteger8);

	renderer.AddOutputImageFormat("EXR - OpenEXR", "exr");
	renderer.AddOutputImageFormatSubType(XSI::siRenderChannelColorType, "RGBA", XSI::siImageBitDepthFloat32);
	renderer.AddOutputImageFormatSubType(XSI::siRenderChannelVectorType, "XYZ", XSI::siImageBitDepthFloat32);

	renderer.AddOutputImageFormat("High Dynamic Range (HDR)", "hdr");
	renderer.AddOutputImageFormatSubType(XSI::siRenderChannelColorType, "RGB", XSI::siImageBitDepthFloat32);

	renderer.AddOutputImageFormat("Bitmap Picture (BMP)", "bmp");
	renderer.AddOutputImageFormatSubType(XSI::siRenderChannelColorType, "RGB", XSI::siImageBitDepthInteger8);

	renderer.AddOutputImageFormat("Truevision (TGA)", "tga");
	renderer.AddOutputImageFormatSubType(XSI::siRenderChannelColorType, "RGBA", XSI::siImageBitDepthInteger8);

	renderer.AddOutputImageFormat("Truevision (TGA)", "tga");
	renderer.AddOutputImageFormatSubType(XSI::siRenderChannelColorType, "RGBA", XSI::siImageBitDepthInteger8);

	renderer.AddOutputImageFormat("Portable Anymap (PPM)", "ppm");
	renderer.AddOutputImageFormatSubType(XSI::siRenderChannelColorType, "RGB", XSI::siImageBitDepthInteger8);

	//setup render channels
	renderer.AddDefaultChannel("ospColor", XSI::siRenderChannelColorType);
	renderer.AddDefaultChannel("ospDepth", XSI::siRenderChannelColorType);
	renderer.AddDefaultChannel("ospAlbedo", XSI::siRenderChannelColorType);
	//vector type output channel
	renderer.AddDefaultChannel("ospNormal", XSI::siRenderChannelVectorType);

	render = &g_render;
	render->set_render_options_name(options_name);

	g_bAborted = false;
	::InitializeCriticalSection(&g_barrierAbort);
	g_hAbort = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	return(XSI::CStatus::OK);
}

SICALLBACK OSPRayRenderer_Term(XSI::CRef &in_ctxt)
{
	::DeleteObject(g_hAbort);
	::DeleteCriticalSection(&g_barrierAbort);

	g_hAbort = NULL;
	::ZeroMemory(&g_barrierAbort, sizeof(g_barrierAbort));

	return(XSI::CStatus::OK);
}

SICALLBACK OSPRayRenderer_Cleanup(XSI::CRef &in_ctxt)
{
	render->clear();
	return(XSI::CStatus::OK);
}

SICALLBACK OSPRayRenderer_Abort(XSI::CRef &in_ctxt)
{
	render->abort();
	set_abort(true);

	return(XSI::CStatus::OK);
}

SICALLBACK OSPRayRenderer_Quality(XSI::CRef &in_ctxt)
{
	return XSI::CStatus::OK;
}

SICALLBACK OSPRayRenderer_Query(XSI::CRef &in_ctxt)
{
	XSI::RendererContext ctxt(in_ctxt);
	const int type = ctxt.GetAttribute("QueryType");
	switch (type)
	{
	case XSI::siRenderQueryArchiveIsValid:
	{
		break;
	}
	case XSI::siRenderQueryWantDirtyList:
	{
		ctxt.PutAttribute("WantDirtyList", true);
		break;
	}
	case XSI::siRenderQueryDisplayBitDepths:
	{
		XSI::CLongArray bitDepths;
		bitDepths.Add(XSI::siImageBitDepthInteger8);
		bitDepths.Add(XSI::siImageBitDepthInteger16);

		XSI::CString softimage_version_string = XSI::Application().GetVersion();
		XSI::CStringArray softimage_version = softimage_version_string.Split(L".");
		if (atoi(softimage_version[0].GetAsciiString()) >= 10)
		{
			bitDepths.Add(XSI::siImageBitDepthFloat32);
		}

		ctxt.PutAttribute("BitDepths", bitDepths);
		break;
	}
	default:;
#if XSISDK_VERSION > 11000
	case XSI::siRenderQueryHasPreMulAlphaOutput:
	{
		ctxt.PutAttribute("HasPreMulAlphaOutput", false);
		break;
	}
#endif      
	}

	return XSI::CStatus::OK;
}

SICALLBACK OSPRAY_OnObjectAdded_OnEvent(XSI::CRef& in_ctxt)
{
	render->on_object_add(in_ctxt);

	return XSI::CStatus::OK;
}


SICALLBACK OSPRAY_OnObjectRemoved_OnEvent(XSI::CRef& in_ctxt)
{
	render->on_object_remove(in_ctxt);

	return XSI::CStatus::OK;
}

XSI::CStatus begin_render_event(XSI::RendererContext& ctxt, XSI::CStringArray& output_paths)
{
	XSI::CStatus status;

	const XSI::siRenderingType render_type = (ctxt.GetSequenceLength() > 1) ? XSI::siRenderSequence : XSI::siRenderFramePreview;
	if (ctxt.GetSequenceIndex() == 0)
	{
		status = ctxt.TriggerEvent(XSI::siOnBeginSequence, render_type, ctxt.GetTime(), output_paths, XSI::siRenderFieldNone);
	}

	if (status != XSI::CStatus::OK)
	{
		return status;
	}
	status = ctxt.TriggerEvent(XSI::siOnBeginFrame, render_type, ctxt.GetTime(), output_paths, XSI::siRenderFieldNone);

	return status;
}

XSI::CStatus end_render_event(XSI::RendererContext& ctxt, XSI::CStringArray& output_paths, bool in_skipped)
{
	XSI::CStatus status;
	const XSI::siRenderingType render_type = (ctxt.GetSequenceLength() > 1) ? XSI::siRenderSequence : XSI::siRenderFramePreview;
	if (!in_skipped)
	{
		status = ctxt.TriggerEvent(XSI::siOnEndFrame, render_type, ctxt.GetTime(), output_paths, XSI::siRenderFieldNone);

		if (status != XSI::CStatus::OK)
		{
			return status;
		}
	}
	if (ctxt.GetSequenceIndex() + 1 == ctxt.GetSequenceLength())
	{
		status = ctxt.TriggerEvent(XSI::siOnEndSequence, render_type, ctxt.GetTime(), output_paths, XSI::siRenderFieldNone);
	}

	return status;
}

SICALLBACK OSPRayRenderer_Process(XSI::CRef &in_ctxt)
{
	set_abort(false);
	XSI::CStatus status;

	XSI::RendererContext ctxt(in_ctxt);
	XSI::Renderer renderer = ctxt.GetSource();

	if (!render->is_ready_to_render())
	{
		return XSI::CStatus::Abort;
	}

	status = render->pre_render(ctxt);  // we return Abort when skip all output images
	if (status == XSI::CStatus::Abort)
	{
		return XSI::CStatus::OK;
	}

	if (status != XSI::CStatus::OK)
	{
		return XSI::CStatus::Abort;
	}

	LockRendererData locker = LockRendererData(renderer);
	if (locker.lock() != XSI::CStatus::OK)
	{
		return XSI::CStatus::Abort;
	}

	render->scene_process();

	if (locker.unlock() != XSI::CStatus::OK)
	{
		render->interrupt_update_scene();
		return  XSI::CStatus::Abort;
	}

	if (is_aborted())
	{
		return XSI::CStatus::Abort;
	}

	status = render->start_render();

	if (status == XSI::CStatus::OK)
	{
		status = render->post_render();
	}

	return status;
}

SICALLBACK OSPRayRendererOptions_Define(XSI::CRef& in_ctxt)
{
	XSI::Context ctxt(in_ctxt);
	XSI::CustomProperty prop = ctxt.GetSource();

	return render->render_option_define(prop);
}

SICALLBACK OSPRayRendererOptions_DefineLayout(XSI::CRef& in_ctxt)
{
	XSI::Context ctxt(in_ctxt);
	return render->render_option_define_layout(ctxt);
}

SICALLBACK OSPRayRendererOptions_PPGEvent(const XSI::CRef& in_ctxt)
{
	XSI::PPGEventContext ctx(in_ctxt);
	return render->render_options_update(ctx);
}