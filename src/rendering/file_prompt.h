/*
* Credit: https://learn.microsoft.com/en-us/windows/win32/learnwin32/example--the-open-dialog-box
*/

#ifndef FILE_PROMPT_H
#define FILE_PROMPT_H

#include <windows.h>
#include <shobjidl.h> 

std::string GetFileDialog()
{
  HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
    COINIT_DISABLE_OLE1DDE);
  if (SUCCEEDED(hr))
  {
    IFileOpenDialog* pFileOpen;

    // Create the FileOpenDialog object.
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
      IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

    if (SUCCEEDED(hr))
    {
      // Show the Open dialog box.
      hr = pFileOpen->Show(NULL);

      // Get the file name from the dialog box.
      if (SUCCEEDED(hr))
      {
        IShellItem* pItem;
        hr = pFileOpen->GetResult(&pItem);
        if (SUCCEEDED(hr))
        {
          PWSTR pszFilePath;
          hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

          // Display the file name to the user.
          if (SUCCEEDED(hr))
          {
            int strLength
              = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1,
                nullptr, 0, nullptr, nullptr);

            std::string str(strLength, 0);

            WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1, &str[0],
              strLength, nullptr, nullptr);

            return str;
          }
          pItem->Release();
        }
      }
      pFileOpen->Release();
    }
    CoUninitialize();
  }
  return "";
}

#endif // !FILE_PROMPT_H