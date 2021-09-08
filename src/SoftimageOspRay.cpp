#include "version.h"
#include "render_osp\includes_osp.h"

#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#include <xsi_application.h>

using namespace XSI; 

SICALLBACK XSILoadPlugin(PluginRegistrar& in_reg)
{
	//add plugin directory to the PATH, because some apps require it for loading libraries
#ifdef _WINDOWS
	// get plugin_path and remove trailing slash
	CString plugin_path = in_reg.GetOriginPath();
	plugin_path.TrimRight("\\");

	// get PATH env
	char *pValue;
	size_t envBufSize;
	int err = _dupenv_s(&pValue, &envBufSize, "PATH");
	if (err)
	{
		Application().LogMessage("Failed to retrieve PATH environment.", siErrorMsg);
	}
	else
	{
		const CString currentPath = pValue;
		free(pValue);

		// check so that plugin_path isn't already in PATH
		if (currentPath.FindString(plugin_path) == UINT_MAX)
		{
			// add plugin_path to beginning of PATH
			CString envPath = plugin_path + ";" + currentPath;

			// set the new path
			err = _putenv_s("PATH", envPath.GetAsciiString());
			if (err)
			{
				Application().LogMessage("Failed to add ospRay path to PATH environment.", siErrorMsg);
			}
		}
	}
#endif
	
	in_reg.PutAuthor("Shekn");
	in_reg.PutName("OSPRay Renderer");
	in_reg.PutVersion(get_major_version(), get_minor_version());
	//RegistrationInsertionPoint - do not remove this line
	in_reg.RegisterRenderer("OSPRay Renderer");           // The renderer
	in_reg.RegisterProperty("OSPRay Renderer Options");   // Render options

	in_reg.RegisterEvent("OSPRAY_OnObjectAdded", siOnObjectAdded);
	in_reg.RegisterEvent("OSPRAY_OnObjectRemoved", siOnObjectRemoved);

	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + " has been unloaded.", siVerboseMsg);
	return CStatus::OK;
}

