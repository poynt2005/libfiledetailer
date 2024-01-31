interface DetailMapping {
  [key: string]: string;
}

interface FileDetails {
  idDefinedValue: DetailMapping;
  idUndefinedValue: DetailMapping;
}

interface NativeBinding {
  FileDetailer_Create: () => string;
  FileDetailer_Free: (handle: string) => void;
  FileDetailer_GetFileDetail: (handle: string, filePath: string) => void;
  FileDetailer_GetLastDetails: (handle: string) => FileDetails;
}
