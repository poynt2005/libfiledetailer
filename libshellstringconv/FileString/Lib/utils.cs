using System.Runtime.InteropServices;

namespace FileString.Lib
{
    public static class Utils
    {
        [DllImport("Urlmon.dll", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern Int64 URLDownloadToFile(IntPtr pCaller, string szURL, string szFileName, UInt64 dwReserved, IntPtr lpfnCB);


        public static bool DownloadFile(string url, string fileName)
        {
            var hResult = URLDownloadToFile
            (
                IntPtr.Zero,
                url,
                fileName,
                0,
                IntPtr.Zero
            );

            return hResult == 0x00000000;
        }

        public static void MoveDir(string src, string dst)
        {
            Action<string, string>? mover = null;

            mover = new((rootDir, outDir) =>
            {

                if (!Directory.Exists(outDir))
                {
                    Directory.CreateDirectory(outDir);
                }

                var dirs = Directory.GetDirectories(rootDir);
                var files = Directory.GetFiles(rootDir);

                foreach (var file in files)
                {
                    var destPath = Path.Combine(outDir, Path.GetFileName(file));
                    var srcPath = Path.Combine(rootDir, Path.GetFileName(file));
                    File.Copy(srcPath, destPath);
                }

                foreach (var dir in dirs)
                {
                    var destPath = Path.Combine(outDir, Path.GetFileName(dir));
                    var srcPath = Path.Combine(rootDir, Path.GetFileName(dir));
                    mover?.Invoke(srcPath, destPath);
                }
            });

            mover.Invoke(src, dst);

            Directory.Delete(src, true);
        }
    }
}