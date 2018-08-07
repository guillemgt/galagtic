#include <iostream>
#include <vector>
#include <map>

#include <dirent.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef unsigned char uint8;
struct RgbColor{
    uint8 r, b, g;
};


void write_obj_file(const char name[], unsigned int triangles, float *positions, float *normals, float *tex_coords){
    FILE *wfile = fopen(name, "wb");
    fwrite(&triangles, sizeof(unsigned int), 1, wfile);
    fwrite(positions, sizeof(float), 9*triangles, wfile);
    fwrite(tex_coords, sizeof(float), 6*triangles, wfile);
    fwrite(normals, sizeof(float), 3*triangles, wfile);
    fclose(wfile);
}
void convertObjFile(std::string filename){
    FILE *file = fopen((filename+".obj").c_str(), "r");
    FILE *mfile = fopen((filename+".mtl").c_str(), "r");
    float x, y, z, rf, gf, bf;
    int ix, iy, iz, tx, ty, tz, nox, noy, noz;
    unsigned int triangles[6];
    std::vector<float> temp_vertices;
    std::vector<float> temp_normals;
    std::vector<float> temp_textures;
    std::vector<int> vertices[6];
    std::vector<int> nnormals[6];
    std::vector<int> textures[6];
    std::vector<std::string> vertMaterials[6];
    std::string usemtl;
    std::map<std::string, RgbColor> materials;
    float *positions[6], *normals[6], *tex_coords[6];
    
    const char object_names[6][50] = {"wall_far", "wall", "middle_floor", "intersection_floor", "middle_ceiling", "intersection_ceiling"};
    
    while(1){
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(mfile, "%s", lineHeader);
        if (res == EOF)
            break;
        if ( strcmp( lineHeader, "Kd" ) == 0 ){
            fscanf(mfile, "%f %f %f\n", &rf, &gf, &bf );
            float gamma = 1 / 2.2;
            rf = pow(rf, gamma);
            gf = pow(gf, gamma);
            bf = pow(bf, gamma);
            materials[usemtl].r = (uint8)(255*rf);
            materials[usemtl].g = (uint8)(255*gf);
            materials[usemtl].b = (uint8)(255*bf);
        }else if(strcmp(lineHeader, "newmtl") == 0){
            char um[128];
            fscanf(mfile, "%s\n", um);
            usemtl = std::string(um);
            printf("\tNew %s\n", um);
        }
    }
    int useobj = 0;
    while(1){
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;
        if ( strcmp( lineHeader, "o" ) == 0 ){
            char obj_name[100];
            fscanf(file, "%s", obj_name);
            for(int obj=0; obj<6; obj++){
                int l = strlen(object_names[obj]);
                int ok = 1;
                for(int i=0; i<l; i++){
                    if(obj_name[i] != object_names[obj][i]){
                        ok = 0;
                        break;
                    }
                }
                if(ok){
                    useobj = obj;
                    printf("Object %s\n", object_names[obj]);
                    break;
                }
            }
        }else if ( strcmp( lineHeader, "v" ) == 0 ){
            fscanf(file, "%f %f %f\n", &x, &z, &y );
            temp_vertices.push_back(x);
            temp_vertices.push_back(-y);
            temp_vertices.push_back(z);
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            fscanf(file, "%f %f %f\n", &x, &z, &y );
            temp_normals.push_back(x);
            temp_normals.push_back(-y);
            temp_normals.push_back(z);
        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
            fscanf(file, "%f %f\n", &x, &y);
            temp_textures.push_back(x);
            temp_textures.push_back(y);
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &ix, &tx, &nox, &iy, &ty, &noy, &iz, &tz, &noz);
            vertices[useobj].push_back(ix-1);
            vertices[useobj].push_back(iy-1);
            vertices[useobj].push_back(iz-1);
            textures[useobj].push_back(tx-1);
            textures[useobj].push_back(ty-1);
            textures[useobj].push_back(tz-1);
            nnormals[useobj].push_back(nox-1);
            vertMaterials[useobj].push_back(usemtl);
        }else if(strcmp(lineHeader, "usemtl") == 0){
            char um[128];
            fscanf(file, "%s\n", um);
            usemtl = std::string(um);
            printf("\tUse %s\n", um);
        }
    }
    fclose(file);
    fclose(mfile);
    
    for(int obj=0; obj<6; obj++){
        triangles[obj] = (unsigned int)vertices[obj].size()/3;
        positions[obj] = (float *)malloc(9*triangles[obj]*sizeof(float));
        normals[obj] = (float *)malloc(3*triangles[obj]*sizeof(float));
        tex_coords[obj] = (float *)malloc(6*triangles[obj]*sizeof(float));
        for(int i=0, j=0, l=0, t=0, j2=0, l2=0; i<triangles[obj]; i++){
            positions[obj][j++] = temp_vertices[3*vertices[obj][l]];
            positions[obj][j++] = temp_vertices[3*vertices[obj][l]+1];
            positions[obj][j++] = temp_vertices[3*vertices[obj][l++]+2];
            positions[obj][j++] = temp_vertices[3*vertices[obj][l]];
            positions[obj][j++] = temp_vertices[3*vertices[obj][l]+1];
            positions[obj][j++] = temp_vertices[3*vertices[obj][l++]+2];
            positions[obj][j++] = temp_vertices[3*vertices[obj][l]];
            positions[obj][j++] = temp_vertices[3*vertices[obj][l]+1];
            positions[obj][j++] = temp_vertices[3*vertices[obj][l++]+2];
            //printf("%f %f %f\n%f %f %f\n%f %f %f\n\n", sq->positions[j-9], sq->positions[j-8], sq->positions[j-7], sq->positions[j-6], sq->positions[j-5], sq->positions[j-4], sq->positions[j-3], sq->positions[j-2], sq->positions[j-1]);
            tex_coords[obj][j2++] = temp_textures[2*textures[obj][l2]];
            tex_coords[obj][j2++] = temp_textures[2*textures[obj][l2++]+1];
            tex_coords[obj][j2++] = temp_textures[2*textures[obj][l2]];
            tex_coords[obj][j2++] = temp_textures[2*textures[obj][l2++]+1];
            tex_coords[obj][j2++] = temp_textures[2*textures[obj][l2]];
            tex_coords[obj][j2++] = temp_textures[2*textures[obj][l2++]+1];
            
            normals[obj][t++] = temp_normals[3*nnormals[obj][i]];
            normals[obj][t++] = temp_normals[3*nnormals[obj][i]+1];
            normals[obj][t++] = temp_normals[3*nnormals[obj][i]+2];
        }
    }
    
    { // Move wall_far
        for(int k=0, j=0, j2=0; k<triangles[0]; k++){
            for(int l=0; l<3; l++){
                positions[0][j] = -3.f-positions[0][j];
                j+=3;
            }
            normals[0][j2] = -normals[0][j2];
            j2+=3;
        }
    }
    for(int obj=0; obj<2; obj++){ // Walls
        if(obj == 1){
            for(int k=0, j=0, j2=0; k<triangles[obj]; k++){
                // Change vertex order
                float tx, ty, tz;
                tx = positions[obj][j+0];
                ty = positions[obj][j+1];
                tz = positions[obj][j+2];
                positions[obj][j+0] = positions[obj][j+3];
                positions[obj][j+1] = positions[obj][j+4];
                positions[obj][j+2] = positions[obj][j+5];
                positions[obj][j+3] = tx;
                positions[obj][j+4] = ty;
                positions[obj][j+5] = tz;
                j += 9;
                
                tx = tex_coords[obj][j2+0];
                ty = tex_coords[obj][j2+1];
                tex_coords[obj][j2+0] = tex_coords[obj][j2+2];
                tex_coords[obj][j2+1] = tex_coords[obj][j2+3];
                tex_coords[obj][j2+2] = tx;
                tex_coords[obj][j2+3] = ty;
                j2 += 6;
            }
        }
        
        printf("\tWriting %s: %i triangles\n", object_names[obj], triangles[obj]);
        for(int i=0; i<4; i++){
            char extension[10];
            sprintf(extension, "%s_%i.model", object_names[obj], i);
            write_obj_file(extension, triangles[obj], positions[obj], normals[obj], tex_coords[obj]);
            
            for(int k=0, j=0, j2=0; k<triangles[obj]; k++){
                for(int l=0; l<3; l++){
                    float a = positions[obj][j]-2.5f;
                    float b = positions[obj][j+1]-2.5f;
                    positions[obj][j++] = -b+2.5f;
                    positions[obj][j++] =  a+2.5f;
                    j++;
                }
                float na = normals[obj][j2];
                float nb = normals[obj][j2+1];
                normals[obj][j2++] = -nb;
                normals[obj][j2++] =  na;
                j2++;
            }
        }
        free(positions[obj]);
        free(tex_coords[obj]);
        free(normals[obj]);
    }
    { // Middle floor
        int obj = 2;
        printf("\tWriting %s: %i triangles\n", object_names[obj], triangles[obj]);
        for(int i=0; i<2; i++){
            for(int k=0, j=0, j2=0; k<triangles[obj]; k++){
                // Change vertex order
                float tx, ty, tz;
                tx = positions[obj][j+0];
                ty = positions[obj][j+1];
                tz = positions[obj][j+2];
                positions[obj][j+0] = positions[obj][j+3];
                positions[obj][j+1] = positions[obj][j+4];
                positions[obj][j+2] = positions[obj][j+5];
                positions[obj][j+3] = tx;
                positions[obj][j+4] = ty;
                positions[obj][j+5] = tz;
                j += 9;
                
                tx = tex_coords[obj][j2+0];
                ty = tex_coords[obj][j2+1];
                tex_coords[obj][j2+0] = tex_coords[obj][j2+2];
                tex_coords[obj][j2+1] = tex_coords[obj][j2+3];
                tex_coords[obj][j2+2] = tx;
                tex_coords[obj][j2+3] = ty;
                j2 += 6;
            }
            
            char extension[10];
            sprintf(extension, "%s_%c.model", object_names[obj], i == 0 ? 'y' : 'x');
            write_obj_file(extension, triangles[obj], positions[obj], normals[obj], tex_coords[obj]);
            
            for(int k=0, j=0, j2=0; k<triangles[obj]; k++){
                int j0 = j;
                for(int l=0; l<3; l++){
                    float a = positions[obj][j];
                    float b = positions[obj][j+1];
                    positions[obj][j++] = b;
                    positions[obj][j++] = a;
                    j++;
                }
                
                float na = normals[obj][j2];
                float nb = normals[obj][j2+1];
                normals[obj][j2++] = nb;
                normals[obj][j2++] = na;
                j2++;
            }
        }
        free(positions[obj]);
        free(tex_coords[obj]);
        free(normals[obj]);
    }
    { // Middle ceiling
        int obj = 4;
        printf("\tWriting %s: %i triangles\n", object_names[obj], triangles[obj]);
        for(int i=0; i<2; i++){
            for(int k=0, j=0, j2=0; k<triangles[obj]; k++){
                // Change vertex order
                float tx, ty, tz;
                tx = positions[obj][j+0];
                ty = positions[obj][j+1];
                tz = positions[obj][j+2];
                positions[obj][j+0] = positions[obj][j+3];
                positions[obj][j+1] = positions[obj][j+4];
                positions[obj][j+2] = positions[obj][j+5];
                positions[obj][j+3] = tx;
                positions[obj][j+4] = ty;
                positions[obj][j+5] = tz;
                j += 9;
                
                tx = tex_coords[obj][j2+0];
                ty = tex_coords[obj][j2+1];
                tex_coords[obj][j2+0] = tex_coords[obj][j2+2];
                tex_coords[obj][j2+1] = tex_coords[obj][j2+3];
                tex_coords[obj][j2+2] = tx;
                tex_coords[obj][j2+3] = ty;
                j2 += 6;
            }
            
            char extension[10];
            sprintf(extension, "%s_%c.model", object_names[obj], i == 0 ? 'y' : 'x');
            write_obj_file(extension, triangles[obj], positions[obj], normals[obj], tex_coords[obj]);
            
            for(int k=0, j=0, j2=0; k<triangles[obj]; k++){
                int j0 = j;
                for(int l=0; l<3; l++){
                    float a = positions[obj][j];
                    float b = positions[obj][j+1];
                    positions[obj][j++] = b;
                    positions[obj][j++] = a;
                    j++;
                }
                
                float na = normals[obj][j2];
                float nb = normals[obj][j2+1];
                normals[obj][j2++] = nb;
                normals[obj][j2++] = na;
                j2++;
            }
        }
        free(positions[obj]);
        free(tex_coords[obj]);
        free(normals[obj]);
    }
    { // Intersection floor
        int obj = 3;
        printf("\tWriting %s: %i triangles\n", object_names[obj], triangles[obj]);
        char extension[10];
        sprintf(extension, "%s.model", object_names[obj]);
        write_obj_file(extension, triangles[obj], positions[obj], normals[obj], tex_coords[obj]);
        free(positions[obj]);
        free(tex_coords[obj]);
        free(normals[obj]);
    }
    { // Intersection ceiling
        int obj = 5;
        
        for(int k=0, j=0, j2=0; k<triangles[obj]; k++){
            // Change vertex order
            float tx, ty, tz;
            tx = positions[obj][j+0];
            ty = positions[obj][j+1];
            tz = positions[obj][j+2];
            positions[obj][j+0] = positions[obj][j+3];
            positions[obj][j+1] = positions[obj][j+4];
            positions[obj][j+2] = positions[obj][j+5];
            positions[obj][j+3] = tx;
            positions[obj][j+4] = ty;
            positions[obj][j+5] = tz;
            j += 9;
            
            tx = tex_coords[obj][j2+0];
            ty = tex_coords[obj][j2+1];
            tex_coords[obj][j2+0] = tex_coords[obj][j2+2];
            tex_coords[obj][j2+1] = tex_coords[obj][j2+3];
            tex_coords[obj][j2+2] = tx;
            tex_coords[obj][j2+3] = ty;
            j2 += 6;
        }
        
        printf("\tWriting %s: %i triangles\n", object_names[obj], triangles[obj]);
        char extension[10];
        sprintf(extension, "%s.model", object_names[obj]);
        write_obj_file(extension, triangles[obj], positions[obj], normals[obj], tex_coords[obj]);
        free(positions[obj]);
        free(tex_coords[obj]);
        free(normals[obj]);
    }
}


int main(int argc, const char * argv[]) {
    std::string path = "./";
    DIR* dirFile = opendir(path.c_str());
    if ( dirFile )
    {
        struct dirent* hFile;
        errno = 0;
        while (( hFile = readdir( dirFile )) != NULL )
        {
            if ( !strcmp( hFile->d_name, "."  )) continue;
            if ( !strcmp( hFile->d_name, ".." )) continue;
            
            // in linux hidden files all start with '.'
            if (  hFile->d_name[0] == '.' ) continue;
            
            // dirFile.name is the name of the file. Do whatever string comparison
            // you want here. Something like:
            if ( strstr( hFile->d_name, ".obj" )){
                std::string s = path+hFile->d_name;
                printf( "Converting %s\n", s.c_str() );
                convertObjFile(s.substr(0, s.size()-4));
            }
        }
        closedir( dirFile );
    }
    return 0;
}
