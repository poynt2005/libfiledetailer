var path = require("path");

var fdIncl = path
  .resolve("../libfiledetailer/libfiledetailer")
  .replace(/\\/gm, "\\\\");

var fdLib = path
  .resolve("../libfiledetailer/x64/Release/libfiledetailer.lib")
  .replace(/\\/gm, "\\\\");

module.exports = {
  fdLib: fdLib,
  fdIncl: fdIncl,
};
