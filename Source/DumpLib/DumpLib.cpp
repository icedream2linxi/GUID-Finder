#include <atlbase.h>
#include <atlcom.h>
#include <atlcomcli.h>
#include <comdef.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

using namespace std;
using namespace ATL;
namespace fs = std::filesystem;

#define INTERFACE_FILENAME "NewInterfaces.txt"
#define CLASS_FILENAME "NewClasses.txt"

#define VERIFHR(hrfn)        \
    if (FAILED((hr = hrfn))) \
    _com_issue_error(hr)

inline std::wstring to_lower_copy(const std::wstring& str)
{
    std::wstring ret;
    for (auto ch : str) {
        ret.append(1, tolower(ch));
    }
    return ret;
}

void DumpFile(map<wstring, pair<bool, wstring>>& guids, const fs::path& filePath)
{
    wcout << L"Dump " << filePath << endl;

    HRESULT hr = S_OK;
    ITypeLibPtr spTypeLib;
    VERIFHR(LoadTypeLibEx(filePath.wstring().c_str(), REGKIND_NONE, &spTypeLib));

    UINT uiTypeInfoCount = spTypeLib->GetTypeInfoCount();
    for (UINT i = 0; i < uiTypeInfoCount; ++i) {
        TYPEKIND typeKind;
        VERIFHR(spTypeLib->GetTypeInfoType(i, &typeKind));
        if (typeKind != TKIND_COCLASS && typeKind != TKIND_DISPATCH && typeKind != TKIND_INTERFACE)
            continue;

        ITypeInfoPtr spTypeInfo;
        VERIFHR(spTypeLib->GetTypeInfo(i, &spTypeInfo));

        _bstr_t bstrName;
        VERIFHR(spTypeInfo->GetDocumentation(MEMBERID_NIL, bstrName.GetAddress(), NULL, NULL, NULL));

        TYPEATTR* pTypeAttr = nullptr;
        VERIFHR(spTypeInfo->GetTypeAttr(&pTypeAttr));

        wchar_t buffer[256];
        if (StringFromGUID2(pTypeAttr->guid, buffer, 256) == 0)
            continue;

        wstring strGuid = (buffer + 1);
        strGuid.pop_back();

        spTypeInfo->ReleaseTypeAttr(pTypeAttr);
        pTypeAttr = nullptr;

        auto iter = guids.find(strGuid);
        if (iter != guids.end())
            continue;

        guids.insert(make_pair(strGuid, make_pair(typeKind == TKIND_COCLASS, (LPCWSTR)bstrName)));
    }
}

int main(int argc, char** argv)
{

    wcout.imbue(locale(""));

    try {
        fs::path filePath;

        if (argc <= 1)
            filePath = fs::current_path();
        else
            filePath = argv[1];
        if (!fs::exists(filePath)) {
            wcout << L"File not exists! " << filePath << endl;
            return -2;
        }

        map<fs::path, map<wstring, pair<bool, wstring>>> guids;
        if (fs::is_directory(filePath)) {
            fs::recursive_directory_iterator endIter;
            for (fs::recursive_directory_iterator iter(filePath); iter != endIter; ++iter) {
                if (fs::is_directory(*iter))
                    continue;

                auto ext = to_lower_copy(iter->path().extension().wstring());
                if (ext != L".dll" && ext != L".exe" && ext != L".idl" && ext != L".ocx" && ext != L".tlb" && ext != L".dat")
                    continue;

                try {
                    DumpFile(guids[*iter], *iter);
                } catch (const _com_error& e) {
                    if (e.Error() != TYPE_E_CANTLOADLIBRARY) {
                        wcout << L"FAILED : " << *iter << endl;
                        wcout << e.ErrorMessage() << endl;
                    }
                }
            }
        } else {
            try {
                DumpFile(guids[filePath], filePath);
            } catch (const _com_error& e) {
                wcout << L"FAILED : " << filePath << endl;
                wcout << e.ErrorMessage() << endl;
            }
        }

        map<wstring, pair<bool, wstring>> uniqueGuids;
        {
            wofstream fout(L"guids.txt");
            for (auto& item : guids) {
                fout << L"# " << item.first << endl;
                for (auto& item2 : item.second) {
                    fout << item2.first << L" " << item2.second.second << endl;

                    if (uniqueGuids.count(item2.first) == 0)
                        uniqueGuids.insert(item2);
                }
            }

            fout << endl;
        }

        {
            wofstream foutClasses(CLASS_FILENAME);
            wofstream foutInterfaces(INTERFACE_FILENAME);

            for (auto& item : uniqueGuids) {
                if (item.second.first) {
                    foutClasses << item.first << L" " << item.second.second << endl;
                } else {
                    foutInterfaces << item.first << L" " << item.second.second << endl;
                }
            }
        }
    } catch (const _com_error& e) {
        wcout << e.ErrorMessage() << endl;
    }

    cout << "finish!!!" << endl;

    return 0;
}
