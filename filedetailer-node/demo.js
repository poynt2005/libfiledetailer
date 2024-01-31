process.chdir("./dist");
var FileDetailerCore = require("./dist/FileDetailer");
var path = require("path");
var fs = require("fs");
var readline = require("readline");

var rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
});

rl.question(
  "[FD][Info ] Please enter the path of file you want to check detail with\n",
  (answer) => {
    var srcFileAbsPath = path.resolve(answer.trim());

    if (!fs.existsSync(srcFileAbsPath)) {
      throw new Error(`file path you entered: ${srcFileAbsPath} is not exists`);
    }

    rl.close();

    var fd = new FileDetailerCore();
    fd.GetFileDetail(srcFileAbsPath);
    console.log(
      "[FD][Info ] File details with be written to dist/result.json\n"
    );
    var result = fd.GetLastDetails();
    fs.writeFileSync("result.json", JSON.stringify(result, null, 4), "utf-8");
    fd.Dispose();
  }
);
