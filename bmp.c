/* Tells the compiler not to add padding for these structs. This may
   be useful when reading/writing to binary files.
   http://stackoverflow.com/questions/3318410/pragma-pack-effect
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1)

typedef struct 
{
    unsigned char  fileMarker1; /* 'B' */
    unsigned char  fileMarker2; /* 'M' */
    unsigned int   bfSize; /* File's size */
    unsigned short unused1; /* Aplication specific */
    unsigned short unused2; /* Aplication specific */
    unsigned int   imageDataOffset; /* Offset to the start of image data */
} bmp_fileheader;

typedef struct 
{
    unsigned int   biSize; /* Size of the info header - 40 bytes */
    signed int     width; /* Width of the image */
    signed int     height; /* Height of the image */
    unsigned short planes;
    unsigned short bitPix; /* Number of bits per pixel = 3 * 8 (for each channel R, G, B we need 8 bits */
    unsigned int   biCompression; /* Type of compression */
    unsigned int   biSizeImage; /* Size of the image data */
    int            biXPelsPerMeter;
    int            biYPelsPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
} bmp_infoheader;

typedef struct 
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} Pixel;

#pragma pack()


void black_white(char *input, char *output1)
{
    FILE *f1, *f2; int x;
    Pixel **matrix;

    bmp_fileheader *h1;
    bmp_infoheader *h2;

    h1 = (bmp_fileheader *)malloc(sizeof(bmp_fileheader));
    h2 = (bmp_infoheader *)malloc(sizeof(bmp_infoheader));

    f1 = fopen(input, "rb");
    f2 = fopen(output1, "wb");

    fread(h1, sizeof(bmp_fileheader), 1, f1);
    fread(h2, sizeof(bmp_infoheader), 1, f1);

    matrix = (Pixel **)malloc(sizeof(Pixel *) * h2->height);
    for(int i = 0; i < h2->height; i++)
        matrix[i] = (Pixel *)malloc(sizeof(Pixel) * h2->width);

    int padding;
    if(((h2->width * h2->bitPix / 8) % 4) != 0)
        padding = 4 - ((h2->width * h2->bitPix / 8) % 4);
    else
        padding = 0;


    for(int i = 0; i < h2->height; i++)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux;
            fread(&aux, sizeof(Pixel), 1, f1);
            matrix[i][j] = aux;
        }
        fseek(f1, padding, SEEK_CUR);
    }


    for(int i = 0; i < h2->height; i++)
        for(int j = 0; j < h2->width; j++)
        {   
            x = (matrix[i][j].blue + matrix[i][j].green + matrix[i][j].red) / 3;
            matrix[i][j].blue = x;
            matrix[i][j].green = x;
            matrix[i][j].red = x;
        }

    fwrite(h1, sizeof(bmp_fileheader), 1, f2);
    fwrite(h2, sizeof(bmp_infoheader), 1, f2);

    for(int i = 0; i < h2->height; i++)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux;
            aux = matrix[i][j];
            fwrite(&aux, sizeof(Pixel), 1, f2);
        }

        for(int k = 0; k < padding; k++)
            fputc(0x00, f2);
    }

    for(int i = 0; i < h2->height; i++)
        free(matrix[i]);
    
    fclose(f1); fclose(f2), free(matrix), free(h1), free(h2);
}

void no_crop(char *input, char *output2)
{
    FILE *f1, *f2;
    Pixel **matrix;
    Pixel *image = NULL;
    Pixel *big_image = NULL;
    int i, k, aux, image_size;

    bmp_fileheader *h1;
    bmp_infoheader *h2;

    h1 = (bmp_fileheader *)malloc(sizeof(bmp_fileheader));
    h2 = (bmp_infoheader *)malloc(sizeof(bmp_infoheader));

    f1 = fopen(input, "rb");
    f2 = fopen(output2, "wb");

    fread(h1, sizeof(bmp_fileheader), 1, f1);
    fread(h2, sizeof(bmp_infoheader), 1, f1);
    image_size = h2->height * h2->width * 3;

    image = (Pixel *)malloc(image_size);
    matrix = (Pixel **)malloc(sizeof(Pixel *) * h2->height);
    for(int i = 0; i < h2->height; i++)
        matrix[i] = (Pixel *)malloc(sizeof(Pixel) * h2->width);


    int padding;
    if(((h2->width * h2->bitPix / 8) % 4) != 0)
        padding = 4 - ((h2->width * h2->bitPix / 8) % 4);
    else
        padding = 0;


    for(int i = 0; i < h2->height; i++)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux;
            fread(&aux, sizeof(Pixel), 1, f1);
            matrix[i][j] = aux;
        }
        fseek(f1, padding, SEEK_CUR);
    }

    for(int i = 0; i < h2->height; i++)
        for(int j = 0; j < h2->width; j++)
            image[i * h2->width + j] = matrix[i][j];

    for(int i = 0; i < h2->height; i++)
        free(matrix[i]);
    free(matrix);

    aux = image_size;

    // determinam maximul dintre lungimea si latimea imaginii
    int max = h2->width;
    if(h2->height > max)
        max = h2->height;

    if(max == h2->width) // completam inaltimea
    {
        image_size = max * max * 3;
        big_image = (Pixel *)malloc(image_size);
    
        if((h2->width - h2->height) % 2 == 0)
        {
            for(i = 0; i < h2->width * ((h2->width - h2->height) / 2); i++)
            {
                big_image[i].blue = 255;
                big_image[i].green = 255;
                big_image[i].red = 255;
            }
    
            for(i = h2->width * ((h2->width - h2->height) / 2), k = 0; i < h2->width * (h2->height + (h2->width - h2->height) / 2), k < h2->width * h2->height; i++, k++)
            {
                big_image[i].blue = image[k].blue;
                big_image[i].green = image[k].green;
                big_image[i].red = image[k].red;
            }

            for(i = h2->width * (h2->height + (h2->width - h2->height) / 2); i < h2->width * h2->width; i++)
            {
                big_image[i].blue = 255;
                big_image[i].green = 255;
                big_image[i].red = 255;
            } 
        }

        else
        {
            for(i = 0; i < h2->width * ((h2->width - h2->height) / 2 + 1); i++)
            {
                big_image[i].blue = 255;
                big_image[i].green = 255;
                big_image[i].red = 255;
            }
    
            for(i = h2->width * ((h2->width - h2->height) / 2 + 1), k = 0; i < h2->width * (h2->height + (h2->width - h2->height) / 2 + 1), k < h2->width * h2->height; i++, k++)
            {
                big_image[i].blue = image[k].blue;
                big_image[i].green = image[k].green;
                big_image[i].red = image[k].red;
            }

            for(i = h2->width * (h2->height + (h2->width - h2->height) / 2 + 1); i < h2->width * h2->width; i++)
            {
                big_image[i].blue = 255;
                big_image[i].green = 255;
                big_image[i].red = 255;
            } 

        }    
        h2->height = h2->width;
    }

    else if(max == h2->height) // completam latimea
    {
        image_size = max * max * 3;
        big_image = (Pixel *)malloc(image_size);

        if((h2->height - h2->width) % 2 == 0)
        {
            for(int i = 0; i < h2->height; i++)
                for(int j = 0; j < (h2->height - h2->width) / 2; j++)
                {
                    big_image[i * h2->height + j].blue = 255; // BIG_IMAGE[I][J];
                    big_image[i * h2->height + j].green = 255; 
                    big_image[i * h2->height + j].red = 255;
                }
    
            for(int i = 0; i < h2->height; i++)
                for(int j = (h2->height - h2->width) / 2, k = 0; j < (h2->height - h2->width) / 2 + h2->width, k < h2->width; j++, k++)
                {
                    big_image[i * h2->height + j].blue = image[i * h2->width + k].blue;
                    big_image[i * h2->height + j].green = image[i * h2->width + k].green;
                    big_image[i * h2->height + j].red = image[i * h2->width + k].red;
                } 

            for(int i = 0; i < h2->height; i++)
                for(int j = (h2->height - h2->width) / 2 + h2->width; j < h2->height; j++)
                {
                    big_image[i * h2->height + j].blue = 255;
                    big_image[i * h2->height + j].green = 255; 
                    big_image[i * h2->height + j].red = 255;
                }
        }

        else
        {
            for(int i = 0; i < h2->height; i++)
                for(int j = 0; j < (h2->height - h2->width) / 2 + 1; j++)
                {
                    big_image[i * h2->height + j].blue = 255;
                    big_image[i * h2->height + j].green = 255; 
                    big_image[i * h2->height + j].red = 255;
                }
    
            for(int i = 0; i < h2->height; i++)
                for(int j = (h2->height - h2->width) / 2, k = 0; j < (h2->height - h2->width) / 2 + h2->width + 1, k < h2->width; j++, k++)
                {
                    big_image[i * h2->height + j].blue = image[i * h2->width + k].blue;
                    big_image[i * h2->height + j].green = image[i * h2->width + k].green;
                    big_image[i * h2->height + j].red = image[i * h2->width + k].red;
                } 

            for(int i = 0; i < h2->height; i++)
                for(int j = (h2->height - h2->width) / 2 + h2->width + 1; j < h2->height; j++)
                {
                    big_image[i * h2->height + j].blue = 255;
                    big_image[i * h2->height + j].green = 255; 
                    big_image[i * h2->height + j].red = 255;
                }
        }


        h2->width = h2->height;
    }

    image_size = aux;
    fwrite(h1, sizeof(bmp_fileheader), 1, f2);
    fwrite(h2, sizeof(bmp_infoheader), 1, f2);

    Pixel **matrix2;

    matrix2 = (Pixel **)malloc(sizeof(Pixel *) * h2->height);
    for(int i = 0; i < h2->height; i++)
        matrix2[i] = (Pixel *)malloc(sizeof(Pixel) * h2->width);

    for(int i = 0; i < h2->height; i++)
        for(int j = 0; j < h2->width; j++)
            matrix2[i][j] = big_image[i * h2->width + j];


    for(int i = 0; i < h2->height; i++)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux;
            aux = matrix2[i][j];
            fwrite(&aux, sizeof(Pixel), 1, f2);
        }

        for(int k = 0; k < padding; k++)
            fputc(0x00, f2);
    }

    for(int i = 0; i < h2->height; i++)
        free(matrix2[i]);

    fclose(f1); fclose(f2), free(matrix2), free(image), free(big_image), free(h1), free(h2);
}

void convolutional_filter(char *input, char *input2, char *output3)
{

    FILE *f1, *f2; int x;
    Pixel **matrix;

    bmp_fileheader *h1;
    bmp_infoheader *h2;

    h1 = (bmp_fileheader *)malloc(sizeof(bmp_fileheader));
    h2 = (bmp_infoheader *)malloc(sizeof(bmp_infoheader));

    f1 = fopen(input, "rb");
    f2 = fopen(output3, "wb");

    fread(h1, sizeof(bmp_fileheader), 1, f1);
    fread(h2, sizeof(bmp_infoheader), 1, f1);

    matrix = (Pixel **)malloc(sizeof(Pixel *) * h2->height);
    for(int i = 0; i < h2->height; i++)
        matrix[i] = (Pixel *)malloc(sizeof(Pixel) * h2->width);

    int padding;
    if(((h2->width * h2->bitPix / 8) % 4) != 0)
        padding = 4 - ((h2->width * h2->bitPix / 8) % 4);
    else
        padding = 0;


    for(int i = h2->height - 1; i >= 0; i--)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux2;
            fread(&aux2, sizeof(Pixel), 1, f1);
            matrix[i][j] = aux2;
        }
        fseek(f1, padding, SEEK_CUR);
    }


    // for(int i = h2->height - 1, k = 0; i >= 0, k < h2->height; i--, k++)
    //     for(int j = 0; j < h2->width; j++)
    //     {
    //         matrix[i][j].blue = image[k * h2->width + j].blue;
    //         matrix[i][j].green = image[k * h2->width + j].green;
    //         matrix[i][j].red = image[k * h2->width + j].red;
    //     }

    // for(int i = 0; i < h2->height; i++)
    //     for(int j = 0; j < h2->width; j++)
    //     {
    //         image[i * h2->width + j].blue = matrix[i][j].blue;
    //         image[i * h2->width + j].green = matrix[i][j].green;
    //         image[i * h2->width + j].red = matrix[i][j].red;
    //     }

    // pana aici avem matricea de pixeli. prelucram pe ea dupa o rescriem in image si scriem image
    
    // for(int i = 0; i < h2->height; i++)
    // {
    //     for(int j = 0; j < h2->width; j++)
    //         printf("(%d %d %d) ", matrix[i][j].blue, matrix[i][j].green, matrix[i][j].red);
    //     printf("\n");
    // }

    int i, j, k, l, y, z;
    ////// citim fisierul de input unde vom avea dimensiunea matricii si matricea in sine (matricea de filtru)
    FILE *f;
    f = fopen(input2, "r");

    int dim_filtru;
    fscanf(f, "%d", &dim_filtru);

    int **matrix_filtru = (int **)malloc(sizeof(int *) * dim_filtru);
    for(int i = 0; i < dim_filtru; i++)
        matrix_filtru[i] = (int *)malloc(sizeof(int) * dim_filtru);
    
    for(int i = 0; i < dim_filtru; i++)
        for(int j = 0; j < dim_filtru; j++)
            if(!fscanf(f, "%d", &matrix_filtru[i][j]))
                break;

    fclose(f);        

    // for(i = 0; i < dim_filtru; i++)
    //     {
    //     for(j = 0; j < dim_filtru; j++)
    //          printf("%d ", matrix_filtru[i][j]);
    //      printf("\n");
    //     }

    Pixel **aux, **copy;

    if(dim_filtru == 3)
    {
        //Pixel aux[h2->height + 2][h2->width + 2], copy[h2->height][h2->width]; // aux va fi matricea cu 0 uri in margine iar copy va fi matricea finala pe care o scriem in fisier

        aux = (Pixel **)malloc(sizeof(Pixel *) * (h2->height + 2));
        for(i = 0; i < (h2->height + 2); i++)
            aux[i] = (Pixel *)malloc(sizeof(Pixel) * (h2->width + 2));

        copy = (Pixel **)malloc(sizeof(Pixel *) * h2->height);
        for(i = 0; i < h2->height; i++)
            copy[i] = (Pixel *)malloc(sizeof(Pixel) * h2->width);


        int sum[3]; // sum[0] = blue, sum[1] = green, sum[2] = red, am facut vector si nu structura pt ca la structura daca depaseste 255 da o valoare random.
        for(int i = 0; i < h2->width + 2; i++)
        {
            aux[0][i].blue = 0;
            aux[0][i].green = 0;
            aux[0][i].red = 0;

            aux[h2->height + 1][i].blue = 0;
            aux[h2->height + 1][i].green = 0;
            aux[h2->height + 1][i].red = 0;
        }

        for(int i = 0; i < h2->height + 2; i++)
        {
            aux[i][0].blue = 0;
            aux[i][0].green = 0;
            aux[i][0].red = 0;

            aux[i][h2->width + 1].blue = 0;
            aux[i][h2->width + 1].green = 0;
            aux[i][h2->width + 1].red = 0;
        }


        for(i = 1, k = 0; i < h2->height + 1 , k < h2->height; i++, k++)
            for(j = 1, l = 0; j < h2->width + 1, l < h2->width; j++, l++)
            {
                aux[i][j].blue = matrix[k][l].blue;
                aux[i][j].green = matrix[k][l].green;
                aux[i][j].red = matrix[k][l].red;
            }

        // for(i = 0; i < h2->height + 2; i++)
        // {
        //     for(j = 0; j < h2->width + 2; j++)
        //     {
        //         printf("(%d %d %d) ", aux[i][j].blue, aux[i][j].green, aux[i][j].red);
        //         printf("%d %d\n", i, j);
        //     }
        //     printf("\n");
        // }

        for(i = 1; i <= h2->height; i++)
        {
            for(j = 1; j <= h2->width; j++)
            {
                sum[0] = 0;
                sum[1] = 0;
                sum[2] = 0;

                for(k = i - 1, y = 0; k <= i + 1, y < dim_filtru; k++, y++)
                {
                     for(l = j - 1, z = 0; l <= j + 1, z < dim_filtru; l++, z++)
                    {
                        sum[0] = sum[0] + aux[k][l].blue * matrix_filtru[y][z];
                        sum[1] = sum[1] + aux[k][l].green * matrix_filtru[y][z];
                        sum[2] = sum[2] + aux[k][l].red * matrix_filtru[y][z];
                        //printf("(%d %d %d)", aux[k][l].blue, aux[k][l].green, aux[k][l].red);
                        //printf("%d ", matrix_filtru[y][z]);
                    }
                }
                //printf("\n");
                if(sum[0] > 255)
                    sum[0] = 255;

                if(sum[1] > 255)
                    sum[1] = 255;

                if(sum[2] > 255)
                    sum[2] = 255;

                if (sum[0] < 0)
                    sum[0] = 0;

                if (sum[1] < 0)
                    sum[1] = 0;

                if (sum[2] < 0)
                    sum[2] = 0;

                //printf("(%d %d %d)\n", sum[0], sum[1], sum[2]);
                copy[i - 1][j - 1].blue = sum[0];
                copy[i - 1][j - 1].green = sum[1];
                copy[i - 1][j - 1].red = sum[2];
            }    
        }

        // for(i = 0; i < h2->height; i++)
        // {
        //     for(j = 0; j < h2->width; j++)
        //         printf("(%d %d %d) ", copy[i][j].blue, copy[i][j].green, copy[i][j].red);
        //     printf("\n");
        // }

        for(i = 0; i < (h2->height + 2); i++)
            free(aux[i]);
        free(aux); 
    }

    else if(dim_filtru == 5)
    {
        //Pixel aux[h2->height + 4][h2->width + 4], copy[h2->height][h2->width]; // aux va fi matricea cu 0 uri in margine iar copy va fi matricea finala pe care o scriem in fisier

        aux = (Pixel **)malloc(sizeof(Pixel *) * (h2->height + 4));
        for(i = 0; i < (h2->height + 4); i++)
            aux[i] = (Pixel *)malloc(sizeof(Pixel) * (h2->width + 4));

        copy = (Pixel **)malloc(sizeof(Pixel *) * h2->height);
        for(i = 0; i < h2->height; i++)
            copy[i] = (Pixel *)malloc(sizeof(Pixel) * h2->width);

        int sum[3]; // sum[0] = blue, sum[1] = green, sum[2] = red, am facut vector si nu structura pt ca la structura daca depaseste 255 da o valoare random.
        for(int i = 0; i < h2->width + 4; i++)
        {
            aux[0][i].blue = 0;
            aux[0][i].green = 0;
            aux[0][i].red = 0;

            aux[1][i].blue = 0;
            aux[1][i].green = 0;
            aux[1][i].red = 0;

            aux[h2->height + 2][i].blue = 0;
            aux[h2->height + 2][i].green = 0;
            aux[h2->height + 2][i].red = 0;

            aux[h2->height + 3][i].blue = 0;
            aux[h2->height + 3][i].green = 0;
            aux[h2->height + 3][i].red = 0;
        }

        for(int i = 0; i < h2->height + 4; i++)
        {
            aux[i][0].blue = 0;
            aux[i][0].green = 0;
            aux[i][0].red = 0;

            aux[i][1].blue = 0;
            aux[i][1].green = 0;
            aux[i][1].red = 0;

            aux[i][h2->width + 2].blue = 0;
            aux[i][h2->width + 2].green = 0;
            aux[i][h2->width + 2].red = 0;

            aux[i][h2->width + 3].blue = 0;
            aux[i][h2->width + 3].green = 0;
            aux[i][h2->width + 3].red = 0;
        }


        for(i = 2, k = 0; i < h2->height + 2 , k < h2->height; i++, k++)
            for(j = 2, l = 0; j < h2->width + 2, l < h2->width; j++, l++)
            {
                aux[i][j].blue = matrix[k][l].blue;
                aux[i][j].green = matrix[k][l].green;
                aux[i][j].red = matrix[k][l].red;
            }

        // for(i = 0; i < h2->height + 4; i++)
        // {
        //     for(j = 0; j < h2->width + 4; j++)
        //     {
        //         printf("(%d %d %d) ", aux[i][j].blue, aux[i][j].green, aux[i][j].red);
        //         //printf("%d %d\n", i, j);
        //     }
        //     printf("\n");
        // }

        for(i = 2; i <= h2->height + 1; i++)
        {
            for(j = 2; j <= h2->width + 1; j++)
            {
                sum[0] = 0;
                sum[1] = 0;
                sum[2] = 0;

                for(k = i - 2, y = 0; k <= i + 2, y < dim_filtru; k++, y++)
                {
                     for(l = j - 2, z = 0; l <= j + 2, z < dim_filtru; l++, z++)
                    {
                        sum[0] = sum[0] + aux[k][l].blue * matrix_filtru[y][z];
                        sum[1] = sum[1] + aux[k][l].green * matrix_filtru[y][z];
                        sum[2] = sum[2] + aux[k][l].red * matrix_filtru[y][z];
                        //printf("(%d %d %d)", aux[k][l].blue, aux[k][l].green, aux[k][l].red);
                        //printf("%d ", matrix_filtru[y][z]);
                    }
                }
                //printf("\n");
                if(sum[0] > 255)
                    sum[0] = 255;

                if(sum[1] > 255)
                    sum[1] = 255;

                if(sum[2] > 255)
                    sum[2] = 255;

                if (sum[0] < 0)
                    sum[0] = 0;

                if (sum[1] < 0)
                    sum[1] = 0;

                if (sum[2] < 0)
                    sum[2] = 0;

                //printf("(%d %d %d)\n", sum[0], sum[1], sum[2]);
                copy[i - 2][j - 2].blue = sum[0];
                copy[i - 2][j - 2].green = sum[1];
                copy[i - 2][j - 2].red = sum[2];
            }    
        }

        // for(i = 0; i < h2->height; i++)
        // {
        //     for(j = 0; j < h2->width; j++)
        //         printf("(%d %d %d) ", copy[i][j].blue, copy[i][j].green, copy[i][j].red);
        //     printf("\n");
        // }

        for(i = 0; i < (h2->height + 4); i++)
           free(aux[i]);
        free(aux);

    }
     

    // for(i = 0, k = h2->height - 1; i < h2->height, k >= 0; i++, k--)
    //     for(j = 0; j < h2->width; j++)
    //     {
    //         image[k * h2->width + j].blue = copy[i][j].blue;
    //         image[k * h2->width + j].green = copy[i][j].green;
    //         image[k * h2->width + j].red = copy[i][j].red;
    //     }

    // for(i = 0; i < h2->height * h2->width; i++)
    //     printf("(%d %d %d) ", image[i].blue, image[i].green, image[i].red);

    fwrite(h1, sizeof(bmp_fileheader), 1, f2);
    fwrite(h2, sizeof(bmp_infoheader), 1, f2);

    for(int i = h2->height - 1; i >= 0; i--)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux2;
            aux2 = copy[i][j];
            fwrite(&aux2, sizeof(Pixel), 1, f2);
        }

        for(int k = 0; k < padding; k++)
            fputc(0x00, f2);
    }

    for(int i = 0; i < h2->height; i++)
        free(matrix[i]);

    for(int i = 0; i < dim_filtru; i++)
       free(matrix_filtru[i]);

    for(i = 0; i < h2->height; i++)
        free(copy[i]);


    fclose(f1); fclose(f2), free(matrix), free(h1), free(h2), free(copy), free(matrix_filtru);
}

void pooling(char *input, char *input3, char *output4)
{
    FILE *f1, *f2; int x;
    Pixel **matrix;

    bmp_fileheader *h1;
    bmp_infoheader *h2;

    h1 = (bmp_fileheader *)malloc(sizeof(bmp_fileheader));
    h2 = (bmp_infoheader *)malloc(sizeof(bmp_infoheader));

    f1 = fopen(input, "rb");
    f2 = fopen(output4, "wb");

    fread(h1, sizeof(bmp_fileheader), 1, f1);
    fread(h2, sizeof(bmp_infoheader), 1, f1);

    matrix = (Pixel **)malloc(sizeof(Pixel *) * h2->height);
    for(int i = 0; i < h2->height; i++)
        matrix[i] = (Pixel *)malloc(sizeof(Pixel) * h2->width);

    int padding;
    if(((h2->width * h2->bitPix / 8) % 4) != 0)
        padding = 4 - ((h2->width * h2->bitPix / 8) % 4);
    else
        padding = 0;


    for(int i = h2->height - 1; i >= 0; i--)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux2;
            fread(&aux2, sizeof(Pixel), 1, f1);
            matrix[i][j] = aux2;
        }
        fseek(f1, padding, SEEK_CUR);
    }

    // for(int i = h2->height - 1, k = 0; i >= 0, k < h2->height; i--, k++)
    //     for(int j = 0; j < h2->width; j++)
    //     {
    //         matrix[i][j].blue = image[k * h2->width + j].blue;
    //         matrix[i][j].green = image[k * h2->width + j].green;
    //         matrix[i][j].red = image[k * h2->width + j].red;
    //     }

    // for(int i = 0; i < h2->height; i++)
    //     for(int j = 0; j < h2->width; j++)
    //     {
    //         image[i * h2->width + j].blue = matrix[i][j].blue;
    //         image[i * h2->width + j].green = matrix[i][j].green;
    //         image[i * h2->width + j].red = matrix[i][j].red;
    //     }

    // pana aici avem matricea de pixeli. prelucram pe ea dupa o rescriem in image si scriem image
    
    // for(int i = 0; i < h2->height; i++)
    // {
    //     for(int j = 0; j < h2->width; j++)
    //         printf("(%d %d %d) ", matrix[i][j].blue, matrix[i][j].green, matrix[i][j].red);
    //     printf("\n");
    // }

    int i, j, k, l, y, z;
    FILE *f;
    f = fopen(input3, "r");

    char m;
    fscanf(f, "%c", &m);

    int dim_filtru;
    fscanf(f, "%d", &dim_filtru);
    fclose(f);        


    Pixel **aux, **copy;

    aux = (Pixel **)malloc(sizeof(Pixel *) * (h2->height + dim_filtru - 1));
        for(i = 0; i < (h2->height + dim_filtru - 1); i++)
            aux[i] = (Pixel *)malloc(sizeof(Pixel) * (h2->width + dim_filtru - 1));

        copy = (Pixel **)malloc(sizeof(Pixel *) * h2->height);
        for(i = 0; i < h2->height; i++)
            copy[i] = (Pixel *)malloc(sizeof(Pixel) * h2->width);


        int max[3], min[3]; // sum[0] = blue, sum[1] = green, sum[2] = red, am facut vector si nu structura pt ca la structura daca depaseste 255 da o valoare random.
        for(int i = 0; i < h2->width + dim_filtru - 1; i++)
        {
            for(int j = 0; j < (dim_filtru - 1) / 2; j++)
            {
                aux[j][i].blue = 0;
                aux[j][i].green = 0;
                aux[j][i].red = 0;
            }

            for(int j = (dim_filtru - 1) / 2; j < dim_filtru - 1; j++)
            {
                aux[h2->height + j][i].blue = 0;
                aux[h2->height + j][i].green = 0;
                aux[h2->height + j][i].red = 0;
            }
            
        }

        for(int i = 0; i < h2->height + dim_filtru - 1; i++)
        {
            for(int j = 0; j < (dim_filtru - 1) / 2; j++)
            {
                aux[i][j].blue = 0;
                aux[i][j].green = 0;
                aux[i][j].red = 0;
            }

            for(int j = (dim_filtru - 1) / 2; j < dim_filtru - 1; j++)
            {
                aux[i][h2->width + j].blue = 0;
                aux[i][h2->width + j].green = 0;
                aux[i][h2->width + j].red = 0; 
            }
            
        }


        for(i = (dim_filtru - 1) / 2 , k = 0; i < h2->height + ((dim_filtru - 1) / 2) , k < h2->height; i++, k++)
            for(j = (dim_filtru - 1) / 2, l = 0; j < h2->width + ((dim_filtru - 1) / 2), l < h2->width; j++, l++)
            {
                aux[i][j].blue = matrix[k][l].blue;
                aux[i][j].green = matrix[k][l].green;
                aux[i][j].red = matrix[k][l].red;
            }

        // for(i = 0; i < h2->height + dim_filtru - 1; i++)
        // {
        //     for(j = 0; j < h2->width + dim_filtru - 1; j++)
        //     {
        //         printf("(%d %d %d) ", aux[i][j].blue, aux[i][j].green, aux[i][j].red);
        //         //printf("%d %d\n", i, j);
        //     }
        //     printf("\n");
        // }
        
        if(m == 'M')
        {
            puts("intra aici");
            for(i = (dim_filtru - 1) / 2; i < h2->height + (dim_filtru - 1) / 2; i++)
            {
                for(j = (dim_filtru - 1) / 2; j < h2->width + (dim_filtru - 1) / 2; j++)
                {
                    max[0] = 0;
                    max[1] = 0;
                    max[2] = 0;

                    for(k = i - (dim_filtru - 1) / 2; k <= i + (dim_filtru - 1) / 2; k++)
                    {
                         for(l = j - (dim_filtru - 1) / 2; l <= j + (dim_filtru - 1) / 2; l++)
                        {
                            if(aux[k][l].blue > max[0])
                                max[0] = aux[k][l].blue;

                            if(aux[k][l].green > max[1])
                                max[1] = aux[k][l].green;

                            if(aux[k][l].red > max[2])
                                max[2] = aux[k][l].red;
                            //printf("(%d %d %d)", aux[k][l].blue, aux[k][l].green, aux[k][l].red);
                        }
                    }
                    //printf("\n");

                    //printf("(%d %d %d)\n", sum[0], sum[1], sum[2]);
                    copy[i - (dim_filtru - 1) / 2][j - (dim_filtru - 1) / 2].blue = max[0];
                    copy[i - (dim_filtru - 1) / 2][j - (dim_filtru - 1) / 2].green = max[1];
                    copy[i - (dim_filtru - 1) / 2][j - (dim_filtru - 1) / 2].red = max[2];
                }    
            }
        }

        else if(m == 'm')
        {

            for(i = (dim_filtru - 1) / 2; i < h2->height + (dim_filtru - 1) / 2; i++)
            {
                for(j = (dim_filtru - 1) / 2; j < h2->width + (dim_filtru - 1) / 2; j++)
                {
                    min[0] = 255;
                    min[1] = 255;
                    min[2] = 255;

                    for(k = i - (dim_filtru - 1) / 2; k <= i + (dim_filtru - 1) / 2; k++)
                    {
                         for(l = j - (dim_filtru - 1) / 2; l <= j + (dim_filtru - 1) / 2; l++)
                        {
                            if(aux[k][l].blue < min[0])
                                min[0] = aux[k][l].blue;

                            if(aux[k][l].green < min[1])
                                min[1] = aux[k][l].green;

                            if(aux[k][l].red < min[2])
                                min[2] = aux[k][l].red;
                            //printf("(%d %d %d)", aux[k][l].blue, aux[k][l].green, aux[k][l].red);
                        }
                    }
                    //printf("\n");

                    //printf("(%d %d %d)\n", sum[0], sum[1], sum[2]);
                    copy[i - (dim_filtru - 1) / 2][j - (dim_filtru - 1) / 2].blue = min[0];
                    copy[i - (dim_filtru - 1) / 2][j - (dim_filtru - 1) / 2].green = min[1];
                    copy[i - (dim_filtru - 1) / 2][j - (dim_filtru - 1) / 2].red = min[2];
                }    
            }
        }
        

        // for(i = 0; i < h2->height; i++)
        // {
        //     for(j = 0; j < h2->width; j++)
        //         printf("(%d %d %d) ", copy[i][j].blue, copy[i][j].green, copy[i][j].red);
        //     printf("\n");
        // }
     

    // for(i = 0, k = h2->height - 1; i < h2->height, k >= 0; i++, k--)
    //     for(j = 0; j < h2->width; j++)
    //     {
    //         image[k * h2->width + j].blue = copy[i][j].blue;
    //         image[k * h2->width + j].green = copy[i][j].green;
    //         image[k * h2->width + j].red = copy[i][j].red;
    //     }

    // for(i = 0; i < h2->height * h2->width; i++)
    //     printf("(%d %d %d) ", image[i].blue, image[i].green, image[i].red);

    fwrite(h1, sizeof(bmp_fileheader), 1, f2);
    fwrite(h2, sizeof(bmp_infoheader), 1, f2);

    for(int i = h2->height - 1; i >= 0; i--)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux2;
            aux2 = copy[i][j];
            fwrite(&aux2, sizeof(Pixel), 1, f2);
        }

        for(int k = 0; k < padding; k++)
            fputc(0x00, f2);
    }

    for(int i = 0; i < h2->height; i++)
        free(matrix[i]);

    for(i = 0; i < h2->height; i++)
        free(copy[i]);

    for(i = 0; i < (h2->height + dim_filtru - 1); i++)
        free(aux[i]);


    fclose(f1); fclose(f2), free(matrix), free(h1), free(h2), free(copy), free(aux);
}

void area(int i, int j, int el_l, int el_c, int treshold, Pixel **matrix, int **copy, int **copy2, int height, int width)
{
    if(copy2[i][j] == 0 && (abs(matrix[el_l][el_c].blue - matrix[i][j].blue) + abs(matrix[el_l][el_c].green - matrix[i][j].green) + abs(matrix[el_l][el_c].red - matrix[i][j].red)) <= treshold)
    {
        copy[i][j] = 1;
        copy2[i][j] = 1;

        if(i != height - 1)
        {
            ++i;
            area(i, j, el_l, el_c, treshold, matrix, copy, copy2, height, width);
            --i;
        }

        if(i > 0)
        {
            --i;
            area(i, j, el_l, el_c, treshold, matrix, copy, copy2, height, width);
            ++i;
        }

        if(j != width - 1)
        {
            ++j;
            area(i, j, el_l, el_c, treshold, matrix, copy, copy2, height, width);
            --j;
        }

        if(j > 0)
        {
            --j;
            area(i, j, el_l, el_c, treshold, matrix, copy, copy2, height, width);
            ++j;
        }
    }
}

void clustering(char *input, char *input4, char *output5)
{
    FILE *f1, *f2; int x;
    Pixel **matrix;

    bmp_fileheader *h1;
    bmp_infoheader *h2;

    h1 = (bmp_fileheader *)malloc(sizeof(bmp_fileheader));
    h2 = (bmp_infoheader *)malloc(sizeof(bmp_infoheader));

    f1 = fopen(input, "rb");
    f2 = fopen(output5, "wb");

    fread(h1, sizeof(bmp_fileheader), 1, f1);
    fread(h2, sizeof(bmp_infoheader), 1, f1);

    matrix = (Pixel **)malloc(sizeof(Pixel *) * h2->height);
    for(int i = 0; i < h2->height; i++)
        matrix[i] = (Pixel *)malloc(sizeof(Pixel) * h2->width);

    int padding;
    if(((h2->width * h2->bitPix / 8) % 4) != 0)
        padding = 4 - ((h2->width * h2->bitPix / 8) % 4);
    else
        padding = 0;


    for(int i = h2->height - 1; i >= 0; i--)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux2;
            fread(&aux2, sizeof(Pixel), 1, f1);
            matrix[i][j] = aux2;
        }
        fseek(f1, padding, SEEK_CUR);
    }

    // for(int i = h2->height - 1, k = 0; i >= 0, k < h2->height; i--, k++)
    //     for(int j = 0; j < h2->width; j++)
    //     {
    //         matrix[i][j].blue = image[k * h2->width + j].blue;
    //         matrix[i][j].green = image[k * h2->width + j].green;
    //         matrix[i][j].red = image[k * h2->width + j].red;
    //     }

     // for(int i = 0; i < h2->height; i++)
     //    {
     //        for(int j = 0; j < h2->width; j++)
     //            printf("(%d %d %d) ", matrix[i][j].blue, matrix[i][j].green, matrix[i][j].red);
     //        printf("\n");
     //    }

    // pana aici avem matricea de pixeli. prelucram pe ea dupa o rescriem in image si scriem image

    FILE *f;
    f = fopen(input4, "r");

    int treshold;
    fscanf(f, "%d", &treshold);
    fclose(f);

    int **copy, **copy2;
    int i, j, contor, el_l, el_c, sum0, sum1, sum2, k, l, ok1, ok2, ok3, ok4;

    copy = (int **)calloc(h2->height, sizeof(int *));
        for(i = 0; i < h2->height; i++)
            copy[i] = (int *)calloc(h2->width, sizeof(int));

    copy2 = (int **)calloc(h2->height, sizeof(int *));
        for(i = 0; i < h2->height; i++)
            copy2[i] = (int *)calloc(h2->width, sizeof(int));

    el_l = 0;
    el_c = 0;
    while(1)
    {
        //printf("el_l = %d el_c = %d\n", el_l, el_c);
        sum0 = 0; // blue
        sum1 = 0; // green
        sum2 = 0; // red
        contor = 0;


        i = el_l;
        j = el_c;

        area(i, j, el_l, el_c, treshold, matrix, copy, copy2, h2->height, h2->width);


        for(i = 0; i < h2->height; i++)
        {
           for(j = 0; j < h2->width; j++)
            {
                if(copy[i][j] == 1)
                {
                    contor++;
                    sum0 += matrix[i][j].blue;
                    sum1 += matrix[i][j].green;
                    sum2 += matrix[i][j].red;
                }
            }
        }

        for(i = 0; i < h2->height; i++)
           for(j = 0; j < h2->width; j++)
                if(copy[i][j] == 1)
                {
                    matrix[i][j].blue = sum0 / contor;
                    matrix[i][j].green = sum1 / contor;
                    matrix[i][j].red = sum2 / contor;
                }


        int conditie = 0;
        for(i = 0; i < h2->height; i++)
        {
            for(j = 0; j < h2->width; j++)
                if(copy2[i][j] == 0)
                {
                    conditie = 1;
                    el_l = i;
                    el_c = j;
                    break;
                }
            if(conditie == 1)
                break;
        }

        for(i = 0; i < h2->height; i++)
            for(j = 0; j < h2->width; j++)
                copy[i][j] = 0;

        // for(int i = 0; i < h2->height; i++)
        // {
        //     for(int j = 0; j < h2->width; j++)
        //         printf("(%d %d %d) ", matrix[i][j].blue, matrix[i][j].green, matrix[i][j].red);
        //     printf("\n");
        // }

        int conditie2 = 1;
        for(i = 0; i < h2->height; i++)
        {
            for(j = 0; j < h2->width; j++)
            {
                if(copy2[i][j] == 0)
                {
                    conditie2 = 0;
                    break;
                }
            }
            if(conditie2 == 0)
                break;
        }

        if(conditie2 == 1)
            break;       
    }


    // for(int i = h2->height - 1, k = 0; i >= 0, k < h2->height; i--, k++)
    //     for(int j = 0; j < h2->width; j++)
    //     {
    //         image[k * h2->width + j].blue = matrix[i][j].blue;
    //         image[k * h2->width + j].green = matrix[i][j].green;
    //         image[k * h2->width + j].red = matrix[i][j].red;
    //     }

    fwrite(h1, sizeof(bmp_fileheader), 1, f2);
    fwrite(h2, sizeof(bmp_infoheader), 1, f2);

    for(int i = h2->height - 1; i >= 0; i--)
    {
        for(int j = 0; j < h2->width; j++)
        {
            Pixel aux2;
            aux2 = matrix[i][j];
            fwrite(&aux2, sizeof(Pixel), 1, f2);
        }

        for(int k = 0; k < padding; k++)
            fputc(0x00, f2);
    }

    for(int i = 0; i < h2->height; i++)
        free(matrix[i]);

    for(i = 0; i < h2->height; i++)
        free(copy[i]);

    for(i = 0; i < h2->height; i++)
        free(copy2[i]);


    fclose(f1); fclose(f2), free(matrix), free(h1), free(h2), free(copy), free(copy2);
}


// void do_nothing6(char *input, char *output5)
// {
//     FILE *f1, *f2;
//     Pixel *image = NULL;

//     bmp_fileheader *h1;
//     bmp_infoheader *h2;

//     h1 = (bmp_fileheader *)malloc(sizeof(bmp_fileheader));
//     h2 = (bmp_infoheader *)malloc(sizeof(bmp_infoheader));

//     f1 = fopen(input, "rb");
//     f2 = fopen(output5, "wb");

//     fread(h1, sizeof(bmp_fileheader), 1, f1);
//     fread(h2, sizeof(bmp_infoheader), 1, f1);

//     image = (Pixel *)malloc(h2->biSizeImage);
//     fread(image, sizeof(Pixel), h2->width * h2->height, f1);

//     Pixel image1[3 * 5 * 3];
//     for(int i = 0; i < 15; i++)
//     {
//         scanf("%d %d %d", &image1[i].blue, &image1[i].green, &image1[i].red);
//     }
//     h2->width = 3;
//     h2->height = 5;

//     fwrite(h1, sizeof(bmp_fileheader), 1, f2);
//     fwrite(h2, sizeof(bmp_infoheader), 1, f2);

//    fwrite(image1, sizeof(Pixel), h2->width * h2->height, f2);
//    fclose(f1); fclose(f2);
// }


int main()
{
    char *input1, *input2, *input3, *input4, *output1, *output2, *output3, *output4, *output5;

    input1 = (char *)malloc(sizeof(char) * 30);
    input2 = (char *)malloc(sizeof(char) * 30);
    input3 = (char *)malloc(sizeof(char) * 30);
    input4 = (char *)malloc(sizeof(char) * 30);

    output1 = (char *)malloc(sizeof(char) * 30);
    output2 = (char *)malloc(sizeof(char) * 30);
    output3 = (char *)malloc(sizeof(char) * 30);
    output4 = (char *)malloc(sizeof(char) * 30);
    output5 = (char *)malloc(sizeof(char) * 30);

    FILE *f;
    f = fopen("input.txt", "r");

    char **line;
    line = (char **)malloc(4 * sizeof(char *));
    for(int i = 0; i < 4; i++)
        line[i] = (char *)malloc(50 * sizeof(char));

    int i = 0;

    while(fgets(line[i], 50, f))
        {
            line[i][strlen(line[i]) - 1] = '\0';
            i++;
            if(i > 3)
                break;
        }


   strcpy(input1, line[0]);
   strcpy(input2, line[1]);
   strcpy(input3, line[2]);
   strcpy(input4, line[3]);

//////////////////////////////////////////////////////////////////////////////
    if(strcmp(input1, "test0.bmp") == 0)
    {
        strcpy(output1, "test0_black_white.bmp");
        strcpy(output2, "test0_nocrop.bmp");
        strcpy(output3, "test0_filter.bmp");
        strcpy(output4, "test0_pooling.bmp");
        strcpy(output5, "test0_clustered.bmp");
    }

    else if(strcmp(input1, "test1.bmp") == 0)
    {
        strcpy(output1, "test1_black_white.bmp");
        strcpy(output2, "test1_nocrop.bmp");
        strcpy(output3, "test1_filter.bmp");
        strcpy(output4, "test1_pooling.bmp");
        strcpy(output5, "test1_clustered.bmp");
    }

    else if(strcmp(input1, "test2.bmp") == 0)
    {
        strcpy(output1, "test2_black_white.bmp");
        strcpy(output2, "test2_nocrop.bmp");
        strcpy(output3, "test2_filter.bmp");
        strcpy(output4, "test2_pooling.bmp");
        strcpy(output5, "test2_clustered.bmp");
    }

    else if(strcmp(input1, "test3.bmp") == 0)
    {
        strcpy(output1, "test3_black_white.bmp");
        strcpy(output2, "test3_nocrop.bmp");
        strcpy(output3, "test3_filter.bmp");
        strcpy(output4, "test3_pooling.bmp");
        strcpy(output5, "test3_clustered.bmp");
    }

    else if(strcmp(input1, "test4.bmp") == 0)
    {
        strcpy(output1, "test4_black_white.bmp");
        strcpy(output2, "test4_nocrop.bmp");
        strcpy(output3, "test4_filter.bmp");
        strcpy(output4, "test4_pooling.bmp");
        strcpy(output5, "test4_clustered.bmp");
    }

    else if(strcmp(input1, "test0.bmp") == 0)
    {
        strcpy(output1, "test0_black_white.bmp");
        strcpy(output2, "test0_nocrop.bmp");
        strcpy(output3, "test0_filter.bmp");
        strcpy(output4, "test0_pooling.bmp");
        strcpy(output5, "test0_clustered.bmp");
    }

    else if(strcmp(input1, "test5.bmp") == 0)
    {
        strcpy(output1, "test5_black_white.bmp");
        strcpy(output2, "test5_nocrop.bmp");
        strcpy(output3, "test5_filter.bmp");
        strcpy(output4, "test5_pooling.bmp");
        strcpy(output5, "test5_clustered.bmp");
    }

    else if(strcmp(input1, "test6.bmp") == 0)
    {
        strcpy(output1, "test6_black_white.bmp");
        strcpy(output2, "test6_nocrop.bmp");
        strcpy(output3, "test6_filter.bmp");
        strcpy(output4, "test6_pooling.bmp");
        strcpy(output5, "test6_clustered.bmp");
    }

    else if(strcmp(input1, "test7.bmp") == 0)
    {
        strcpy(output1, "test7_black_white.bmp");
        strcpy(output2, "test7_nocrop.bmp");
        strcpy(output3, "test7_filter.bmp");
        strcpy(output4, "test7_pooling.bmp");
        strcpy(output5, "test7_clustered.bmp");
    }

    else if(strcmp(input1, "test8.bmp") == 0)
    {
        strcpy(output1, "test8_black_white.bmp");
        strcpy(output2, "test8_nocrop.bmp");
        strcpy(output3, "test8_filter.bmp");
        strcpy(output4, "test8_pooling.bmp");
        strcpy(output5, "test8_clustered.bmp");
    }

    else if(strcmp(input1, "test9.bmp") == 0)
    {
        strcpy(output1, "test9_black_white.bmp");
        strcpy(output2, "test9_nocrop.bmp");
        strcpy(output3, "test9_filter.bmp");
        strcpy(output4, "test9_pooling.bmp");
        strcpy(output5, "test9_clustered.bmp");
    }

//////////////////////////////////////////////////////////////////


    //scanf("%s", output);

    //Pixel *image = foo2(input1);
   // printf("%d %d %d", image[2].blue, image[2].green, image[2].red);
    black_white(input1, output1);
    no_crop(input1, output2);
    convolutional_filter(input1, input2, output3);
    pooling(input1, input3, output4);
    clustering(input1, input4, output5);
    //do_nothing6(input1, output5);

    for(int i = 0; i < 4; i++)
        free(line[i]);

    fclose(f);
    free(line); free(input1); free(input2); free(input3); free(input4);
    free(output1); free(output2); free(output3); free(output4); free(output5); 
    return 0;
}

/*for(int i = 0; i < h2->height; i++)
        for(int j = 0; j < (h2->width - h2->height) / 2; j++)
        {
            matrix[i][j].blue = 255;
            matrix[i][j].green = 255;
            matrix[i][j].red = 255;
        }

    for(int i = 0; i < h2->height; i++)
        for(int j = 0; j < (h2->width - h2->height) / 2; j++)
        {
            //printf("%d\n", i * h2->width + j);
            image[i * h2->width + j].blue = matrix[i][j].blue;
            image[i * h2->width + j].green = matrix[i][j].green; 
            image[i * h2->width + j].red = matrix[i][j].red;
        }
*/

 /*for(int i = 0; i < h2->height; i++)
    {
        for(int j = 0; j < h2->width; j++)
            printf("(%d %d %d) ", matrix[i][j].blue, matrix[i][j].green, matrix[i][j].red);
        printf("\n\n\n\n\n");
    } */


   //  FILE *f1, *f2;
   //  Pixel *image = NULL;

   //  bmp_fileheader *h1;
   //  bmp_infoheader *h2;

   //  h1 = (bmp_fileheader *)malloc(sizeof(bmp_fileheader));
   //  h2 = (bmp_infoheader *)malloc(sizeof(bmp_infoheader));

   //  f1 = fopen(input, "rb");
   //  f2 = fopen(output5, "wb");

   //  fread(h1, sizeof(bmp_fileheader), 1, f1);
   //  fread(h2, sizeof(bmp_infoheader), 1, f1);

   //  image = (Pixel *)malloc(h2->biSizeImage);
   //  fread(image, sizeof(Pixel), h2->width * h2->height, f1);

   //  Pixel image1[4 * 4 * 3];
   //  for(int i = 0; i < 16; i++)
   //  {
   //      scanf("%d %d %d", &image1[i].blue, &image1[i].green, &image1[i].red);
   //  }
   //  h2->width = 4;
   //  h2->height = 4;

   //  fwrite(h1, sizeof(bmp_fileheader), 1, f2);
   //  fwrite(h2, sizeof(bmp_infoheader), 1, f2);

   // fwrite(image1, sizeof(Pixel), h2->width * h2->height, f2);
   // fclose(f1); fclose(f2);