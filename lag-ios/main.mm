#include "main.hpp"

#include "../code/Engine/engine.hpp"

#include "../code/main.cpp"

// ios functions

OsFile openGameFile(const char *name, const char *mode){
    NSString* full_file_name = [NSString stringWithUTF8String:name];
    NSString* file_name = [[full_file_name lastPathComponent] stringByDeletingPathExtension];
    NSString* extension = [full_file_name pathExtension];
    NSString *path = [[NSBundle mainBundle] pathForResource:file_name ofType:extension];
    FILE *fp = fopen([path UTF8String], mode);
    return fp;
}
/*OsFile openUserFile(const char *name, const char *mode){
    NSString *path = [[applicationSupportDirectory() stringByAppendingString:@"/"] stringByAppendingString:[NSString stringWithUTF8String:name]];
    FILE *fp = fopen([path UTF8String], mode);
    return fp;
}*/

OsFile openGameFile(String name, const char *mode){
    return openGameFile((char *)name.text, mode);
}
/*OsFile openUserFile(String name, const char *mode){
    return openUserFile((char *)name.text, mode);
}*/

void getGameFilePath(const char *name, char *dst){
    //const char *path = [[[NSBundle mainBundle] resourcePath] UTF8String];
    //sprintf(dst, "%s/%s", path, name);
    
    NSString* full_file_name = [NSString stringWithUTF8String:name];
    NSString* file_name = [full_file_name stringByDeletingPathExtension];
    NSString* extension = [full_file_name pathExtension];
    NSString *path = [[NSBundle mainBundle] pathForResource:file_name ofType:extension];
    sprintf(dst, "%s", [path UTF8String]);
}
/*void getUserFilePath(const char *name, char *dst){
    const char *path = [applicationSupportDirectory() UTF8String];
    sprintf(dst, "%s/%s", path, name);
}*/

void readFile(void *dest, unsigned int size, unsigned int amount, OsFile fp){
    fread(dest, size, amount, fp);
}
void writeFile(void *dest, unsigned int size, unsigned int amount, OsFile fp){
    fwrite(dest, size, amount, fp);
}
void closeFile(OsFile fp){
    fclose(fp);
}

// Directories
// https://www.cocoawithlove.com/2010/05/finding-or-creating-application-support.html
NSString* findOrCreateDirectory(NSSearchPathDirectory searchPathDirectory, NSSearchPathDomainMask domainMask, NSString* appendComponent, NSError **errorOut){
    // Search for the path
    NSArray* paths = NSSearchPathForDirectoriesInDomains(
                                                         searchPathDirectory,
                                                         domainMask,
                                                         YES);
    if ([paths count] == 0)
    {
        // *** creation and return of error object omitted for space
        return nil;
    }
    
    // Normally only need the first path
    NSString *resolvedPath = [paths objectAtIndex:0];
    
    if (appendComponent)
    {
        resolvedPath = [resolvedPath
                        stringByAppendingPathComponent:appendComponent];
    }
    
    // Create the path if it doesn't exist
    NSError *error;
    BOOL success = [[NSFileManager defaultManager]
                    createDirectoryAtPath:resolvedPath
                    withIntermediateDirectories:YES
                    attributes:nil
                    error:&error];
    if (!success)
    {
        if (errorOut)
        {
            *errorOut = error;
        }
        return nil;
    }
    
    // If we've made it this far, we have a success
    if (errorOut)
    {
        *errorOut = nil;
    }
    return resolvedPath;
}

NSString* applicationSupportDirectory(){
    NSString *executableName =
    [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleExecutable"];
    NSError *error;
    NSString *result = findOrCreateDirectory(NSApplicationSupportDirectory, NSUserDomainMask, executableName, &error);
    if (error)
    {
        NSLog(@"Unable to find or create application support directory:\n%@", error);
    }
    return result;
}

void createUserDirectory(const char *name){
    NSString *path = [[applicationSupportDirectory() stringByAppendingString:@"/"] stringByAppendingString:[NSString stringWithUTF8String:name]];
    NSError *error;
    [[NSFileManager defaultManager]
     createDirectoryAtPath: path
     withIntermediateDirectories:YES
     attributes:nil
     error:&error];
    if (error)
    {
        NSLog(@"Unable to create directory:\n%@", error);
    }
}
