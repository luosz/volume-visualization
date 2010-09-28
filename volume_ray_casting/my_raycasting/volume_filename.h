#ifndef volume_filename_h
#define volume_filename_h

#ifdef MAX_PATH
#define STR_BUFFER_SIZE MAX_PATH
#else
#define STR_BUFFER_SIZE 320
#endif

// filename can be set in command arguments
// in Visual Studio, it is in project Properties->Debugging->Command Arguments
char volume_filename[STR_BUFFER_SIZE] = "data\\nucleon.dat";

/************************************************************************
Common volume data: Tooth Engine foot head256 VisMale CT_Head_small CT_Head_large lobster

Name	Size
marschnerlobb.raw	68 KB
nucleon.raw	68 KB
silicium.raw	111 KB
Bonsai.png	201 KB
Box.raw	256 KB
d4.raw	256 KB
HohlKugel_64x64x64.raw	256 KB
neghip.raw	256 KB
D_5_CTA_inOhr_1_128_char.raw	480 KB
DTI-MD.raw	928 KB
Teddybear.raw	992 KB
DTI-B0.raw	"1,856 KB"
hydrogenAtom.raw	"2,048 KB"
ncat_phantom_segmentation.raw	"2,048 KB"
shockwave.raw	"2,048 KB"
Spheres.raw	"2,048 KB"
walnut.raw	"2,736 KB"
Frog.raw	"2,816 KB"
Monkey-CT.raw	"3,968 KB"
Tomato.raw	"4,096 KB"
lobster.raw	"5,334 KB"
Daisy.raw	"5,670 KB"
Baby.raw	"6,272 KB"
CT_Head_small.raw	"6,784 KB"
phantom_1.0mm_normal_crisp.raw	"6,943 KB"
Engine.raw	"7,040 KB"
mri_ventricles.raw	"7,936 KB"
Clouds.raw	"8,192 KB"
VisMale.raw	"8,192 KB"
mrt8_angio2.raw	"10,240 KB"
statueLeg.raw	"10,561 KB"
BostonTeapot.raw	"11,392 KB"
mouse0.raw	"12,129 KB"
cylinder.raw	"14,400 KB"
head256.raw	"14,400 KB"
aneurism.raw	"16,384 KB"
Bonsai.raw	"16,384 KB"
foot.raw	"16,384 KB"
skull.raw	"16,384 KB"
Bruce.raw	"19,968 KB"
Tooth.raw	"20,608 KB"
CT-Knee.raw	"25,851 KB"
CT_Head_large.raw	"27,136 KB"
Sheep.raw	"30,976 KB"
XmasTree-L0.raw	"31,872 KB"
0.raw	"32,768 KB"
CT-Chest.raw	"34,560 KB"
Bonsai3-HI.raw	"39,424 KB"
Knee.raw	"44,544 KB"
mrt16_angio.raw	"46,592 KB"
Bonsai2-HI.raw	"48,384 KB"
Cadaver.raw	"54,272 KB"
ncat_phantom_medium.raw	"62,500 KB"
Carp.raw	"65,536 KB"
Pig.raw	"68,608 KB"
walnut1.raw	"81,400 KB"
stent16.raw	"89,088 KB"
Bunny.raw	"184,832 KB"
XMasTree.raw	"255,488 KB"
************************************************************************/

void print_about(int argc, char* argv[])
{
	char about[STR_BUFFER_SIZE * 4] = 
"//////////////////////////////////////////////////////////////////////////\n\
GPU raycasting demo \n\
Copyright (c) 2010 Luo Shengzhou, Shenzhen Institute of Advanced Technology, Chinese Academy of Sciences. \n\
\n\
Usage: \n\
%s [source] \n\
source - File to load. \n\
//////////////////////////////////////////////////////////////////////////\n\n";
	char *p = strrchr(argv[0], '\\');
	if (NULL == p)
	{
		p = strrchr(argv[0], '/');
	}
	printf(about, p + 1);
}

#endif // volume_filename_h