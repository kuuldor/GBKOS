#ifndef ABOUTDIALOG_H_INCLUDED
#define ABOUTDIALOG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
	// If pszAppNameVer is NULL, ShowAboutDialog get the name and
	// the version of the application in the about dialog from the
	// current running application's database. So if this function
	// is called from an application, pszAppNameVer can be NULL.
	// But if it is called from a DA, pszAppNameVer must be specified
	// with the correct information of the name and the version.
	//
	// The pszAppNameVer's format is "[AppName][space][space]v[VersionNumber]".
	void ShowAboutDialog(const Char *pszAppNameVer);
	
#ifdef __cplusplus
}
#endif

#endif
