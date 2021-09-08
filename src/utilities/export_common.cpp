#include <xsi_x3dobject.h>
#include <xsi_time.h>
#include <xsi_property.h>

bool is_xsi_object_visible(const XSI::CTime &eval_time, XSI::X3DObject &xsi_object)
{
	XSI::Property visibility_prop;
	xsi_object.GetPropertyFromName("visibility", visibility_prop);
	if (visibility_prop.IsValid())
	{
		return visibility_prop.GetParameterValue("rendvis", eval_time);
	}
	return false;
}