// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the FSKCOREDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// FSKCOREDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef FSKCOREDLL_EXPORTS
#define FSKCOREDLL_API __declspec(dllexport)
#else
#define FSKCOREDLL_API __declspec(dllimport)
#endif

// This class is exported from the dll
class FSKCOREDLL_API CFSKCoredll {
public:
	CFSKCoredll(void);
	// TODO: add your methods here.
};

extern FSKCOREDLL_API int nFSKCoredll;

FSKCOREDLL_API int fnFSKCoredll(void);
