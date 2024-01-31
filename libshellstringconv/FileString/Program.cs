using System.IO.Compression;
using System.Text.RegularExpressions;
using System.Xml;
using FileString.Lib;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;


const string filesRepoZip = "https://github.com/files-community/Files/archive/1e8e43727762cc10ce4cd8b7591c82138a66089d.zip";
const string fileRepoZipName = "files_repo.zip";
const string fileRepoZipFolder = "files_repo";
const string targetResultDirectory = "FileStrings";


if (!File.Exists(fileRepoZipName))
{
    Utils.DownloadFile(filesRepoZip, fileRepoZipName);

    if (!File.Exists(fileRepoZipName))
    {
        throw new Exception($"[FS][Error ] Cannot download files repo from url {filesRepoZip}");
    }
}


if (!Directory.Exists(fileRepoZipName))
{
    ZipFile.ExtractToDirectory(fileRepoZipName, fileRepoZipFolder);
    File.Delete(fileRepoZipName);
    if (!Directory.Exists(fileRepoZipFolder))
    {
        throw new Exception($"[FS][Error ] Cannot extract files repo from zip {fileRepoZipName}");
    }

    var topFileFolder = Directory.GetDirectories(fileRepoZipFolder)[0];

    var targetStringFolder = Path.Combine(Directory.GetCurrentDirectory(), topFileFolder, "src", "Files.App", "Strings");

    if (!Directory.Exists(fileRepoZipFolder))
    {
        throw new Exception($"[FS][Error ] Cannot find target string folder {targetStringFolder}");
    }


    if (!Directory.Exists(targetResultDirectory))
    {
        Utils.MoveDir(targetStringFolder, Path.Combine(Directory.GetCurrentDirectory(), targetResultDirectory));

        if (!Directory.Exists(targetResultDirectory))
        {
            throw new Exception($"[FS][Error ] Move target result directory failed");
        }
    }

    Directory.Delete(fileRepoZipFolder, true);
}


JObject jResult = [];

var dirs = Directory.GetDirectories("FileStrings");


Regex templatePattern = new(@"\{[0-9]+\}");

foreach (var dir in dirs)
{
    JObject shellStringMapping = [];

    DirectoryInfo info = new(dir);

    var resw = info.GetFiles()[0];
    var xmlString = File.ReadAllText(resw.FullName);

    XmlDocument doc = new();
    doc.LoadXml(xmlString);

    var dataTags = doc.GetElementsByTagName("data");

    for (int i = 0; i < dataTags.Count; ++i)
    {
        var tag = dataTags.Item(i);

        var nameAttr = tag.Attributes["name"].InnerText;



        var children = tag.ChildNodes;

        for (int j = 0; j < children.Count; ++j)
        {
            var child = children.Item(j);

            if (child.Name.ToLower() == "value")
            {
                if (templatePattern.IsMatch(child.InnerText))
                {
                    continue;
                }

                shellStringMapping[child.InnerText.Trim()] = nameAttr;
            }
        }
    }

    jResult[Path.GetFileName(dir).Trim()] = shellStringMapping;
}

var resultJson = JsonConvert.SerializeObject(jResult);

File.WriteAllText("shell_mapping.json", resultJson);
Directory.Delete(targetResultDirectory, true);