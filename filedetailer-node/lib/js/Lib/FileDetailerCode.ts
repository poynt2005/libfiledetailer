import { checkRuntimeAndGetNativeBinding } from "./RuntimeChecker";
const nativeBinding: NativeBinding = checkRuntimeAndGetNativeBinding();

class FileDetailerCore {
  /**
   * This is a pointer to instance in native context
   * @memberof FileDetailerCore
   */
  private m_strNativeHandle: string = "0";

  /**
   * Creates a instance, it will also create a handle point to native context
   *
   * @throws Will throw an error with a failure reason if the instance is not created
   * @constructor
   */
  constructor() {
    this.m_strNativeHandle = nativeBinding.FileDetailer_Create();
  }

  /**
   * Remember to call dispose when end of the operation,
   * This method will terminate and free the instance in native context
   * @memberof FileDetailerCore
   */
  public Dispose() {
    if (this.m_strNativeHandle == "0") {
      throw new Error("filedetailer instance has been disposed");
    }

    nativeBinding.FileDetailer_Free(this.m_strNativeHandle);
    this.m_strNativeHandle = "0";
  }

  /**
   * Start to call COM object to obtain details of a specific file
   *
   * @param filePath - Path of file that you want to check
   * @throws Will throw an error with a failure reason if the context returns a error code
   * @memberof FileDetailerCore
   */
  public GetFileDetail(filePath: string) {
    if (this.m_strNativeHandle == "0") {
      throw new Error("instance not created");
    }

    nativeBinding.FileDetailer_GetFileDetail(this.m_strNativeHandle, filePath);
  }

  /**
   * Retrieve last GetFileDetail call result from native context
   *
   * @throws Will throw an error with a failure reason if the context returns a error code
   * @memberof FileDetailerCore
   * @returns { FileDetails } - File details
   */
  public GetLastDetails(): FileDetails {
    if (this.m_strNativeHandle == "0") {
      throw new Error("instance not created");
    }

    const fd: FileDetails = nativeBinding.FileDetailer_GetLastDetails(
      this.m_strNativeHandle
    );

    return fd;
  }
}

export default FileDetailerCore;
