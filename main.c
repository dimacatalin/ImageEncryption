#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    unsigned char  r;
    unsigned char  g;
    unsigned char  b;

} pixel;

typedef struct
{
    int w,h;
    pixel *px;
    unsigned char *head;

} returned;

unsigned int xorshift(unsigned int x)
{
    x=x^x<<13;
    x=x^x>>17;
    x=x^x<<5;
    return x;
}

returned citire(char* fisier)
{
    unsigned char *rgb;
    unsigned int i;
    returned x;
    FILE *file;
    file=fopen(fisier,"rb");
    x.head=malloc(54*sizeof(unsigned char));
    if(file)
        fread(x.head,sizeof(unsigned char),54,file);

    x.w=x.head[19]*256+x.head[18];
    x.h=x.head[23]*256+x.head[22];

    //citire pixeli
    fseek(file,54,SEEK_SET);
    x.px=calloc(x.w*x.h*3,sizeof(unsigned char));
    rgb=calloc(3,sizeof(unsigned char));
    for(i=0;i<x.w*x.h;i++)
    {
        fread(rgb, 3, 1, file);

        x.px[i].b=rgb[0];
        x.px[i].g=rgb[1];
        x.px[i].r=rgb[2];
    }
    free(rgb);
    fclose(file);
    return x;
}

returned criptare(char* fisier, char* secretkey)
{
    returned x;
    x=citire(fisier);
    int i;
    unsigned long long int key1,key2,*a;
    FILE *key;
    key=fopen(secretkey,"r");
    fscanf(key,"%ul %ul",&key1,&key2);

    a=calloc(x.w*x.h*2-1,sizeof(unsigned long long int));

    //xorshift
    for(i=0;i<2*x.w*x.h-1;i++)
    {
        key1=xorshift(key1);
        a[i]=key1;
    }

    //durstenfeld
    unsigned int *v,aux,r;
    v=calloc(x.w*x.h,sizeof(unsigned int));

    for(i=0;i<x.w*x.h;i++)
        v[i]=i;

    for(i=x.w*x.h-1;i>=1;i--)
    {
        r=a[i]%(i+1);
        aux=v[r];
        v[r]=v[i];
        v[i]=aux;
    }
    //permutarea pixelilor
    pixel *ax;
    ax=calloc(x.w*x.h*3,sizeof(unsigned char));
    for(i=0;i<x.w*x.h;i++)
        ax[i]=x.px[v[v[i]]];
    //xorarea
    x.px[0].r=(((key2>>8)>>8)&255)^ax[0].r^(((a[x.w*x.h]>>8)>>8)&255);
    x.px[0].g=((key2>>8)&255)^ax[0].g^((a[x.w*x.h]>>8)&255);
    x.px[0].b=(key2&255)^ax[0].b^(a[x.w*x.h]&255);
     for(i=1;i<x.w*x.h;i++)
     {
        x.px[i].r=x.px[i-1].r^ax[i].r^(((a[x.w*x.h+i]>>8)>>8)&255);
        x.px[i].g=x.px[i-1].g^ax[i].g^((a[x.w*x.h+i]>>8)&255);
        x.px[i].b=x.px[i-1].b^ax[i].b^(a[x.w*x.h+i]&255);
     }
    free(ax);
    free(a);
    free(v);
    fclose(key);
    return x;
}

void afisare(char* output,returned x)
{
    int i;
    FILE* out;
    out=fopen(output,"wb");
    fwrite(x.head,sizeof(unsigned char),54,out);
    fseek(out,54,SEEK_SET);
    for(i=0;i<x.w*x.h;i++)
    {
        fwrite(&x.px[i].b,sizeof(unsigned char),1,out);
        fwrite(&x.px[i].g,sizeof(unsigned char),1,out);
        fwrite(&x.px[i].r,sizeof(unsigned char),1,out);
    }
    fclose(out);
}

returned decriptare(char* secretkey,returned x)
{
    int i;
    unsigned long long int key1,key2,*a;
    FILE *key;
    key=fopen(secretkey,"r");
    fscanf(key,"%ul %ul",&key1,&key2);
    a=calloc(x.w*x.h*2-1,sizeof(unsigned long long int));

    //xorshift
    for(i=0;i<2*x.w*x.h-1;i++)
    {
        key1=xorshift(key1);
        a[i]=key1;
    }

    //durstenfeld
    unsigned int *v,aux,r;
    v=calloc(x.w*x.h,sizeof(unsigned int));

    for(i=0;i<x.w*x.h;i++)
        v[i]=i;

    for(i=x.w*x.h-1;i>=1;i--)
    {
        r=a[i]%(i+1);
        aux=v[r];
        v[r]=v[i];
        v[i]=aux;
    }
    pixel *old;
    old=calloc(x.w*x.h*3,sizeof(unsigned char));
    old[0].r=(((key2>>8)>>8)&255)^x.px[0].r^(((a[x.w*x.h]>>8)>>8)&255);
    old[0].g=((key2>>8)&255)^x.px[0].g^((a[x.w*x.h]>>8)&255);
    old[0].b=(key2&255)^x.px[0].b^(a[x.w*x.h]&255);
     for(i=1;i<x.w*x.h;i++)
     {
        old[i].r=x.px[i-1].r^x.px[i].r^(((a[x.w*x.h+i]>>8)>>8)&255);
        old[i].g=x.px[i-1].g^x.px[i].g^((a[x.w*x.h+i]>>8)&255);
        old[i].b=x.px[i-1].b^x.px[i].b^(a[x.w*x.h+i]&255);
     }

    unsigned int *vp;
    vp=calloc(x.w*x.h,sizeof(unsigned int));
    for(i=0;i<x.w*x.h;i++)
        vp[v[i]]=i;
    for(i=0;i<x.w*x.h;i++)
        x.px[i]=old[vp[vp[i]]];
    fclose(key);
    free(old);
    free(v);
    free(a);
    free(vp);
    return x;
}

void chi(char* fisier)
{
    returned x;
    x=citire(fisier);
    int i;
    int *fr,*fg,*fb;
    fr=calloc(256,sizeof(int));
    fg=calloc(256,sizeof(int));
    fb=calloc(256,sizeof(int));

    for(i=0;i<=255;i++)
    {
        fr[i]=0;
        fg[i]=0;
        fb[i]=0;
    }

    double xr=0,xg=0,xb=0,fp=x.w*x.h/256;
    for(i=0;i<x.w*x.h;i++)
    {
        fr[x.px[i].r]++;
        fg[x.px[i].g]++;
        fb[x.px[i].b]++;
    }
    for(i=0;i<=255;i++)
    {
        xr=xr+((fr[i]-fp)*(fr[i]-fp))/fp;;
        xg=xg+((fg[i]-fp)*(fg[i]-fp))/fp;
        xb=xb+((fb[i]-fp)*(fb[i]-fp))/fp;
    }
    printf("\n R: %lf \n G: %lf \n B: %lf \n",xr,xg,xb);
    free(fr);
    free(fg);
    free(fb);
}

int main()
{
    returned x,y;
    char fisier[] = "peppers.bmp";
    chi(fisier);
    char output[] = "output.bmp";
    char cheiesecreta[] = "secret_key.txt";
    x=criptare(fisier,cheiesecreta);
    afisare(output,x);
    chi(output);
    x=decriptare(cheiesecreta,x);
    afisare(output,x);
}
