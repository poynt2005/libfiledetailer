package main

/*
#include <stdlib.h>
*/
import "C"
import (
	"libshellstringconv/stringconv"
	"unsafe"
)

//export StringConv_GetShellId
func StringConv_GetShellId(szInputShellString *C.char, pszOutputShellString **C.char) C.int {
	shellId, ok := stringconv.ShellLangMapping[C.GoString(szInputShellString)]
	if ok {
		*pszOutputShellString = C.CString(shellId)
		return 1
	}
	return 0
}

//export StringConv_FreeStringBuffer
func StringConv_FreeStringBuffer(ppBuffer **C.char) {
	if ppBuffer == nil || *ppBuffer == nil {
		return
	}

	C.free(unsafe.Pointer(*ppBuffer))
	*ppBuffer = nil
}

func main() {

}
