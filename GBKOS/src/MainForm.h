
#ifndef GBKOS_MAINFORM_H_
#define GBKOS_MAINFORM_H_

#include <PalmOS.h>


/*********************************************************************
 * Helper template functions
 *********************************************************************/

/* use this template like:
 * ControlType *button; GetObjectPtr(button, MainOKButton); */

template <class T>
void GetObjectPtr(T * &ptr, UInt16 id)
{
	FormType * frmP;

	frmP = FrmGetActiveForm();
	ptr = static_cast<T *>(
	          FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, id)));
}

/* use this template like:
 * ControlType *button = 
 *     GetObjectPtr<ControlType>(MainOKButton); */

template <class T>
T * GetObjectPtr(UInt16 id)
{
	FormType * frmP;

	frmP = FrmGetActiveForm();
	return static_cast<T *>(
	           FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, id)));
}
#endif /* GBKOS_MAINFORM_H_ */
