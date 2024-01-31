package stringconv

/*

#cgo LDFLAGS: -lKernel32

#include <Windows.h>
#include <winnls.h>


*/
import "C"

import (
	"embed"
	"fmt"
	"strings"

	"github.com/bitly/go-simplejson"
	"golang.org/x/sys/windows"
)

//go:embed shell_mapping.json
var shellMappingFile embed.FS

var ShellLangMapping map[string]string

func init() {
	file, err := shellMappingFile.Open("shell_mapping.json")

	if err != nil {
		panic("cannot open embed json file")
	}
	defer file.Close()

	sj, err := simplejson.NewFromReader(file)
	if err != nil {
		panic("cannot read the opened json file")
	}

	sjMapper, ok := sj.Interface().(map[string]interface{})

	if !ok {
		panic("cannot type traits to string:json{} type")
	}

	slm, err := getCurrentUILangMapping(sjMapper)

	if err != nil {
		panic(err)
	}

	ShellLangMapping = slm
}

func getCurrentUILangMapping(shellMappingJson map[string]interface{}) (map[string]string, error) {

	getLocalStr := func(dwLcType C.LCTYPE) (string, error) {
		var dwBufLen C.DWORD = 20

		wszLocalString := make([]C.WCHAR, dwBufLen)

		nRet := C.GetLocaleInfoW(C.LOCALE_SYSTEM_DEFAULT, dwLcType, &wszLocalString[0], C.int(dwBufLen))

		if nRet == 0 {
			return "", fmt.Errorf("cannot get local info from windows system")
		}

		localStr := windows.UTF16PtrToString((*uint16)(&wszLocalString[0]))

		return localStr, nil
	}

	localCtryName, err := getLocalStr(C.LOCALE_SISO3166CTRYNAME)

	if err != nil {
		return nil, err
	}

	localLangName, err := getLocalStr(C.LOCALE_SISO639LANGNAME)

	if err != nil {
		return nil, err
	}

	langCode := fmt.Sprintf("%s-%s", localLangName, localCtryName)

	if localCtryName == "CN" || localCtryName == "SG" {
		langCode = "zh-Hans"
	} else if localCtryName == "TW" || localCtryName == "MO" || localCtryName == "HK" {
		langCode = "zh-Hant"
	}

	var langMapping map[string]string = nil

	extractLmType := func(unknownType interface{}) (map[string]string, error) {
		lm, ok := unknownType.(map[string]interface{})

		if !ok {
			return nil, fmt.Errorf("cannot type trait from unknown type to map string:unknown")
		}

		ret := map[string]string{}
		for k, mType := range lm {
			if v, ok := mType.(string); ok {
				ret[k] = v
			}
		}

		return ret, nil
	}

	if lmType, ok := shellMappingJson[langCode]; ok {
		if lm, err := extractLmType(lmType); err == nil {
			langMapping = lm
		}
	} else {
		for lc, lmType := range shellMappingJson {
			langCodeSplit := strings.Split(lc, "-")
			if langCodeSplit[0] == localLangName {
				if lm, err := extractLmType(lmType); err == nil {
					langMapping = lm
				}
			}
		}
	}

	if langMapping == nil {
		return nil, fmt.Errorf("cannot get local mapping info by local languague string %s", langCode)
	}

	return langMapping, nil
}
