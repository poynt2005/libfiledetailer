# libfiledetailer

### 說明

利用 windows 的 Shell.Application COM Object 進行 file 元數據查詢  
並且利用 [Files 項目](https://github.com/files-community/Files) 裡面的字串對應將對應的 shell 名稱 ID 對成英文  
主要使用 node native addon 的模式加載  
輸出的結果分為 non-id 與 id 分別代表 _沒有在 files 被定義的 id_ 以及 _有在 files 被定義的 id_

### 運行環境

1. Microsoft Visual C++ >= 2015
2. NodeJs 20.x

### 建置環境

1. MsBuild v143 (Visual Studio 2022)
2. golang >= 1.21.x
3. python 3.x
4. NodeJs 20.x
5. mingw x64

### 建置

右鍵以 powershell 運行 build.ps1

### 使用

建置完成後，powershell cd 至 filedetailer-node 後  
運行  
`node demo.js`
輸入一個檔名的路徑，需要為檔案，不能為目錄
