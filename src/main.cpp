#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <unistd.h>

#define WIDTH 800
#define HEIGHT 600
/*
 typedef struct SCROLLINFO{
  ScrollInfo.cbSize = sizeof(ScrollInfo);     // size of this structure
  ScrollInfo.fMask = SIF_ALL;                 // parameters to set
  ScrollInfo.nMin = 0;                        // minimum scrolling position
  ScrollInfo.nMax = 100;                      // maximum scrolling position
  ScrollInfo.nPage = 40;                      // the page size of the scroll box
  ScrollInfo.nPos = 50;                       // initial position of the scroll box
  ScrollInfo.nTrackPos = 0;                   // immediate position of a scroll box that the user is dragging
  m_MyScrollBar.SetScrollInfo(&ScrollInfo);
 } ScrollInfo;
*/

int render_amount = 13;
int render_begin = 0;

typedef struct AppData
{
    TTF_Font *font;
    SDL_Texture *phrase;
    SDL_Texture *directory;
    SDL_Texture *executable;
    SDL_Texture *image;
    SDL_Texture *video;
    SDL_Texture *codeFile;
    SDL_Texture *other;
    bool recursive;
    std::string pathOnScreen;
} AppData;

typedef struct listing
{
    std::string name;
    std::string type;
    std::string fullPath;
    std::string permissions;
    std::string size;
    int textx;
    int texty;
    int textw;
    int texth;
    int picx;
    int picy;
    int picw;
    int pich;
} listing;

std::vector<listing> listOnScreen;
void initialize(SDL_Renderer *renderer, AppData *data_ptr);
void render(SDL_Renderer *renderer, AppData *data_ptr, std::vector<listing> *listOnScreen, std::string name);
void quit(AppData *data_ptr);
void showFiles(std::string name, std::vector<listing> *listOnScreen, AppData *data_ptr, SDL_Renderer *renderer, SDL_Rect *rect);
void getPermissionsAndSize(listing *listing_ptr, struct stat *file);
void toOpen(std::string toOpen);

int main(int argc, char **argv)
{
    char *home = getenv("HOME");
    std::string home2(home);
    //printf("HOME: %s\n", home);

    // initializing SDL as Video
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    // create window and renderer
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);

    // initialize and perform rendering loop
    AppData data;
    data.pathOnScreen = home2;

    initialize(renderer, &data);

    SDL_Event event;
    SDL_WaitEvent(&event);
    render(renderer, &data, &listOnScreen, home2);
    

    while (event.type != SDL_QUIT)
    {
        SDL_WaitEvent(&event);
        switch (event.type)
        {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                for (int i = 0; i < listOnScreen.size(); i++)
                {
                    if (event.button.x >= listOnScreen[i].picx && event.button.x <= listOnScreen[i].picx + listOnScreen[i].picw && event.button.y >= listOnScreen[i].picy && event.button.y <= listOnScreen[i].picy + listOnScreen[i].pich)
                    {
                        //user clicked on icon
                        struct stat info;
                        int err = stat(listOnScreen[i].fullPath.c_str(), &info);
                        if (err == 0 && S_ISDIR(info.st_mode))
                        {
                            //icon was for a directory
                            listing temp = listOnScreen.at(i);
                            listOnScreen.clear();
                            std::string copiedPath = temp.fullPath;
                            render(renderer, &data, &listOnScreen, copiedPath);
                        }
                        else
                        {
                            std::string file = listOnScreen.at(i).fullPath;
                            toOpen(file);
                        }
                    }
                    else if (event.button.x >= listOnScreen[i].textx && event.button.x <= listOnScreen[i].textx + listOnScreen[i].textw && event.button.y >= listOnScreen[i].texty && event.button.y <= listOnScreen[i].texty + listOnScreen[i].texth)
                    {
                        //user clicked on file name
                        struct stat info;
                        int err = stat(listOnScreen[i].fullPath.c_str(), &info);
                        if (err == 0 && S_ISDIR(info.st_mode))
                        {
                            //file name was for a directory
                            listing temp = listOnScreen.at(i);
                            listOnScreen.clear();
                            std::string copiedPath = temp.fullPath;
                            render(renderer, &data, &listOnScreen, copiedPath);
                        }
                        else
                        {
                            std::string file = listOnScreen.at(i).fullPath;
                            toOpen(file);
                        }
                    }
                    else
                    {
                        //user clicked on empty space
                    }
                }
                if (event.button.x >= 600 && event.button.x <= 723 && event.button.y >= 20 && event.button.y <= 45)
                {
                    //user clicked on the recursive text to toggle it off/on
                    data.recursive = !(data.recursive);
                    render(renderer, &data, &listOnScreen, data.pathOnScreen);
                }
                if (event.button.x >= 700 && event.button.x <= 900 && event.button.y >= 530 && event.button.y <= 555)
                {
                    //user clicked on the previpo page button
                    render_begin = render_begin + render_amount;
                    render(renderer, &data, &listOnScreen, data.pathOnScreen);
                }
                if (event.button.x >= 20 && event.button.x <= 200 && event.button.y >= 530 && event.button.y <= 555)
                {
                    //user clicked on the next page button
                    render_begin = render_begin - render_amount;
                    render(renderer, &data, &listOnScreen, data.pathOnScreen);
                } 
                

            }
        }
    }

    // clean up
    quit(&data);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
    IMG_Quit();

    return 0;
}

void initialize(SDL_Renderer *renderer, AppData *data_ptr)
{
    //load font
    data_ptr->font = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 18);
    SDL_Surface *directory_surf = IMG_Load("resrc/directory.png");
    data_ptr->directory = SDL_CreateTextureFromSurface(renderer, directory_surf);
    SDL_FreeSurface(directory_surf);

    SDL_Surface *executable_surf = IMG_Load("resrc/executable.png");
    data_ptr->executable = SDL_CreateTextureFromSurface(renderer, executable_surf);
    SDL_FreeSurface(executable_surf);

    SDL_Surface *image_surf = IMG_Load("resrc/image.png");
    data_ptr->image = SDL_CreateTextureFromSurface(renderer, image_surf);
    SDL_FreeSurface(image_surf);

    SDL_Surface *video_surf = IMG_Load("resrc/video.png");
    data_ptr->video = SDL_CreateTextureFromSurface(renderer, video_surf);
    SDL_FreeSurface(video_surf);

    SDL_Surface *codeFile_surf = IMG_Load("resrc/codeFile.png");
    data_ptr->codeFile = SDL_CreateTextureFromSurface(renderer, codeFile_surf);
    SDL_FreeSurface(codeFile_surf);

    SDL_Surface *other_surf = IMG_Load("resrc/other.png");
    data_ptr->other = SDL_CreateTextureFromSurface(renderer, other_surf);
    SDL_FreeSurface(other_surf);
    data_ptr->recursive = false;
}

void render(SDL_Renderer *renderer, AppData *data_ptr, std::vector<listing> *listOnScreen, std::string name)
{
    //SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    SDL_SetRenderDrawColor(renderer, 100, 204, 235, 255);

    // erase renderer content
    SDL_RenderClear(renderer);

    data_ptr->pathOnScreen = name;

    //render recursive 'button'
    SDL_Rect rect;
    rect.x = 600;
    rect.y = 20;
    rect.w = 123;
    rect.h = 25;

    std::string recursiveButtonString = "";
    if (data_ptr->recursive)
    {
        recursiveButtonString = "Recursive: ON";
        render_amount = 2;
    }
    else
    {
        recursiveButtonString = "Recursive: OFF";
        render_amount = 13;
    }
    SDL_Color color = {0, 0, 0};
    SDL_Surface *recursiveButtonSurface = TTF_RenderText_Solid(data_ptr->font, recursiveButtonString.c_str(), color);
    SDL_Texture *recursiveButtonTexture = SDL_CreateTextureFromSurface(renderer, recursiveButtonSurface);
    SDL_FreeSurface(recursiveButtonSurface);
    SDL_QueryTexture(recursiveButtonTexture, NULL, NULL, &(rect.w), &(rect.h));
    SDL_RenderCopy(renderer, recursiveButtonTexture, NULL, &rect);

    //render files on screen
    rect.x = 60;
    rect.y = 60;
    rect.w = 30;
    rect.h = 30;

    showFiles(name, listOnScreen, data_ptr, renderer, &rect);

    SDL_Rect rightRect;
    rightRect.x = 700;
    rightRect.y = 530;
    rightRect.w = 200;
    rightRect.h = 25;

    std::string rightButtonString = "Next Page";
    SDL_Color rightColor = {80, 0, 0};
    SDL_Surface *rightButtonSurface = TTF_RenderText_Solid(data_ptr->font, rightButtonString.c_str(), color);
    SDL_Texture *rightButtonTexture = SDL_CreateTextureFromSurface(renderer, rightButtonSurface);
    SDL_FreeSurface(rightButtonSurface);
    SDL_QueryTexture(rightButtonTexture, NULL, NULL, &(rightRect.w), &(rightRect.h));
    SDL_RenderCopy(renderer, rightButtonTexture, NULL, &rightRect);

    SDL_Rect leftRect;
    leftRect.x = 20;
    leftRect.y = 530;
    leftRect.w = 200;
    leftRect.h = 25;

    std::string leftButtonString = "Previous Page";
    SDL_Color leftColor = {200, 0, 0};
    SDL_Surface *leftButtonSurface = TTF_RenderText_Solid(data_ptr->font, leftButtonString.c_str(), color);
    SDL_Texture *leftButtonTexture = SDL_CreateTextureFromSurface(renderer, leftButtonSurface);
    SDL_FreeSurface(leftButtonSurface);
    SDL_QueryTexture(leftButtonTexture, NULL, NULL, &(leftRect.w), &(leftRect.h));
    SDL_RenderCopy(renderer, leftButtonTexture, NULL, &leftRect);

    // show rendered frame
    SDL_RenderPresent(renderer);
}

void quit(AppData *data_ptr)
{
    SDL_DestroyTexture(data_ptr->codeFile);
    SDL_DestroyTexture(data_ptr->directory);
    SDL_DestroyTexture(data_ptr->executable);
    SDL_DestroyTexture(data_ptr->image);
    SDL_DestroyTexture(data_ptr->other);
    SDL_DestroyTexture(data_ptr->phrase);
    SDL_DestroyTexture(data_ptr->video);
    TTF_CloseFont(data_ptr->font);
}

void showFiles(std::string name, std::vector<listing> *listOnScreen, AppData *data_ptr, SDL_Renderer *renderer, SDL_Rect *rect)
{
    SDL_Rect begRect;
    begRect.x = rect->x;
    begRect.y = rect->y;
    begRect.w = 30;
    begRect.h = 30;

    //only clear the list if recursive mode is off
    if (data_ptr->recursive == false)
    {
        listOnScreen->clear();
    }

    std::vector<std::string> files;

    DIR *dir = opendir(name.c_str());
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        files.push_back(entry->d_name);
    }

    closedir(dir);

    std::sort(files.begin(), files.end());

    listing temp[files.size()];
    SDL_Color color = {0, 0, 0};


    //if the user hit next past the total amount of pages we have to display, go back to the beginning
    if(render_begin > files.size()){
        render_begin = 0;
    }
    //where to end the loop
    int render_end = 0;
    render_end = render_begin + render_amount;
    //if there arent enough files to display a whole page, just display the rest of the file
    //std::cout<<"render_end "<< render_end<< std::endl;
    if(files.size() < render_end){
        render_end = files.size();
    }


    for (int i = render_begin; i < render_end; i++)
    {
        listing tempListing;
        std::string extension = files[i].substr(files[i].find('.') + 1);
        if (files[i] != ".")
        {
            std::string fullFile = name + "/" + files[i];
            struct stat file;
            int err = stat(fullFile.c_str(), &file);
            getPermissionsAndSize(&tempListing, &file);
            tempListing.name = files[i];
            tempListing.fullPath = fullFile;

            if (S_ISDIR(file.st_mode))
            {
                //type is directory
                if (files[i] == "..")
                {
                    std::string test = name.substr(0, name.find_last_of('/'));
                    if (test == "")
                    {
                        tempListing.name = "/";
                        tempListing.fullPath = "/";
                    }
                    else
                    {
                        tempListing.name = name.substr(0, name.find_last_of('/'));
                        tempListing.fullPath = name.substr(0, name.find_last_of('/'));
                    }
                }
                tempListing.type = "directory";
                if (rect->y + rect->w < 600)
                {
                    SDL_RenderCopy(renderer, data_ptr->directory, NULL, rect);
                }
            }
            else if ((S_IEXEC & file.st_mode) != 0)
            {
                //type is executable
                tempListing.type = "executable";
                if (rect->y + rect->w < 600)
                {
                    SDL_RenderCopy(renderer, data_ptr->executable, NULL, rect);
                }
            }
            else if (extension == "jpg" || extension == "jpeg" || extension == "png" || extension == "tif" || extension == "tiff" || extension == "gif")
            {
                //type is image
                tempListing.type = "image";
                if (rect->y + rect->w < 600)
                {
                    SDL_RenderCopy(renderer, data_ptr->image, NULL, rect);
                }
            }
            else if (extension == "mp4" || extension == "mov" || extension == "mkv" || extension == "avi" || extension == "webm")
            {
                //type is video
                tempListing.type = "video";
                if (rect->y + rect->w < 600)
                {
                    SDL_RenderCopy(renderer, data_ptr->video, NULL, rect);
                }
            }
            else if (extension == "h" || extension == "c" || extension == "cpp" || extension == "py" || extension == "java" || extension == "js")
            {
                //type is code file
                tempListing.type = "codeFile";
                if (rect->y + rect->w < 600)
                {
                    SDL_RenderCopy(renderer, data_ptr->codeFile, NULL, rect);
                }
            }
            else
            {
                //type is other
                tempListing.type = "other";
                if (rect->y + rect->w < 600)
                {
                    SDL_RenderCopy(renderer, data_ptr->other, NULL, rect);
                }
            }

            if (rect->y + rect->w < 600)
            {
                //set x,y,w,h of icon and store in listing struct item
                tempListing.picx = rect->x;
                tempListing.picy = rect->y;
                tempListing.pich = rect->h;
                tempListing.picw = rect->w;
                rect->x += 40;
                //render on screen
                SDL_Surface *fileSurface = TTF_RenderText_Solid(data_ptr->font, files[i].c_str(), color);
                SDL_Texture *fileTexture = SDL_CreateTextureFromSurface(renderer, fileSurface);
                SDL_FreeSurface(fileSurface);
                SDL_QueryTexture(fileTexture, NULL, NULL, &(rect->w), &(rect->h));
                SDL_RenderCopy(renderer, fileTexture, NULL, rect);
                // set x,y,w,h of text and store in listing struct item
                tempListing.textx = rect->x;
                tempListing.texty = rect->y;
                tempListing.texth = rect->h;
                tempListing.textw = rect->w;
                //set rect again and print out size and permissions
                getPermissionsAndSize(&tempListing, &file);
                //render permissions and file size of the file
                rect->w = 30;
                rect->h = 30;
                rect->x = 600;
                SDL_Surface *sizeSurface = TTF_RenderText_Solid(data_ptr->font, tempListing.size.c_str(), color);
                SDL_Texture *sizeTexture = SDL_CreateTextureFromSurface(renderer, sizeSurface);
                SDL_FreeSurface(sizeSurface);
                SDL_QueryTexture(sizeTexture, NULL, NULL, &(rect->w), &(rect->h));
                SDL_RenderCopy(renderer, sizeTexture, NULL, rect);
                rect->w = 30;
                rect->h = 30;
                rect->x = 700;
                SDL_Surface *permissionsSurface = TTF_RenderText_Solid(data_ptr->font, tempListing.permissions.c_str(), color);
                SDL_Texture *permissionsTexture = SDL_CreateTextureFromSurface(renderer, permissionsSurface);
                SDL_FreeSurface(permissionsSurface);
                SDL_QueryTexture(permissionsTexture, NULL, NULL, &(rect->w), &(rect->h));
                SDL_RenderCopy(renderer, permissionsTexture, NULL, rect);

                rect->w = 30;
                rect->h = 30;
                rect->x = begRect.x;
                rect->y += 35;

                //add the listing struct item to the list that stores all the files on screen.
                listOnScreen->push_back(tempListing);
            }
            else
            {
                // stop looping through files when file will not be on screen anyore.
                break;
            }

            //if recursive mode is true then recurse on directories that are not ".."
            if (tempListing.type == "directory" && data_ptr->recursive && files[i] != "..")
            {
                std::string fileNameToPass = tempListing.fullPath;
                rect->x = rect->x + 20;
                showFiles(fileNameToPass, listOnScreen, data_ptr, renderer, rect);
            }
        }
    }
}

void getPermissionsAndSize(listing *listing_ptr, struct stat *file)
{
    //permissions
    std::string permissionsString = "";
    permissionsString += ((file->st_mode & S_IRUSR) ? "r" : "-");
    permissionsString += ((file->st_mode & S_IWUSR) ? "w" : "-");
    permissionsString += ((file->st_mode & S_IXUSR) ? "x" : "-");
    permissionsString += ((file->st_mode & S_IRGRP) ? "r" : "-");
    permissionsString += ((file->st_mode & S_IWGRP) ? "w" : "-");
    permissionsString += ((file->st_mode & S_IXGRP) ? "x" : "-");
    permissionsString += ((file->st_mode & S_IROTH) ? "r" : "-");
    permissionsString += ((file->st_mode & S_IWOTH) ? "w" : "-");
    permissionsString += ((file->st_mode & S_IXOTH) ? "x" : "-");
    listing_ptr->permissions = permissionsString;

    //size
    int sizeInt = 0;
    std::string fileSizeString = "";
    if (file->st_size >= 10732109824)
    {
        //GB
        sizeInt = (file->st_size / 10732109824);
        fileSizeString = std::to_string(sizeInt) + " GB";
    }
    else if (file->st_size >= 10480576)
    {
        //MB
        sizeInt = (file->st_size / 10480576);
        fileSizeString = std::to_string(sizeInt) + " MB";
    }
    else if (file->st_size >= 1024)
    {
        //KB
        sizeInt = (file->st_size / 1024);
        fileSizeString = std::to_string(sizeInt) + " KB";
    }
    else
    {
        //B
        sizeInt = (file->st_size);
        fileSizeString = std::to_string(sizeInt) + " B";
    }
    listing_ptr->size = fileSizeString;
}

void toOpen(std::string toOpen)
{
    std::string command = "/usr/bin/xdg-open";
    std::string pls = toOpen;
    std::string c = "xdg-open";
    char *cmd = new char[c.length() + 1];
    char *temp = new char[pls.length() + 1];
    strcpy(temp, pls.c_str());
    strcpy(cmd, c.c_str());
    char *arr[3];
    arr[0] = cmd;
    arr[1] = temp;
    arr[2] = NULL;

    int pid = fork();
    //child process
    if (pid == 0)
    {
        //run executable
        execv(command.c_str(), arr);
    }
}
