//Useful until be gets around to making these sorts of things
//globals akin to be_plain_font, etc.


#ifndef COLORS_H
#define COLORS_H


//******************************************************************************************************
//**** SYSTEM HEADER FILES
//******************************************************************************************************
#include <GraphicsDefs.h>


//******************************************************************************************************
//**** CONSTANT DEFINITIONS
//******************************************************************************************************
//Be standard UI colors
const rgb_color BeBackgroundGrey = 		{216,216,216,	255};
const rgb_color BeInactiveControlGrey = {240,240,240,	255};
const rgb_color BeFocusBlue =			{0,  0,  229,	255};
const rgb_color BeHighlight =			{255,255,255,	255};
const rgb_color BeShadow =				{152,152,152,	255};
const rgb_color BeDarkShadow =			{108,108,108,	255};
const rgb_color BeLightShadow =			{194,194,194,	255};
const rgb_color BeButtonGrey =			{232,232,232,	255};
const rgb_color BeButtonDarkShadow =	{96,96,96,		255};
const rgb_color BeInactiveGrey =		{127,127,127,	255};
const rgb_color BeListSelectGrey =		{178,178,182,	255};
const rgb_color BeListGrey =			{250,250,250,	255};
const rgb_color BeTitleBarYellow =		{255,203,0,		255};

// FramesView
const rgb_color FrameRed=				{128,48,48,		255};
const rgb_color BeFrameListText=		{50,50,50,		255};
const rgb_color FrameLight=				{250,250,250,	255};
const rgb_color BeFrameListGrey =		{230,230,230,	255};
const rgb_color FrameShadow=			{210,210,210,	255};
const rgb_color FrameDarkShadow=		{195,195,195,	255};
const rgb_color CurrentFrameBody =		{197,199,230,	255};
const rgb_color CurrentFrameLight =		{223,226,245,	255};
const rgb_color CurrentFrameShadow =	{175,178,212,	255};
const rgb_color CurrentFrameDarkShadow ={160,163,197,	255};

// ColumnListItem Colors
//const rgb_color ItemShadow=				{210,210,210,	255};
//const rgb_color ItemLight=				{250,250,250,	255};
//const rgb_color ItemBody=				{230,230,230,	255};
const rgb_color ItemShadow=				{210,210,210,	255};
const rgb_color ItemLight=				{250,250,250,	255};
const rgb_color ItemBody=				{238,238,238,	255};

//Other colors
const rgb_color Black =					{0,  0,  0,		255};
const rgb_color White =					{255,255,255,	255};
const rgb_color Red =					{255,0,  0,		255};
const rgb_color Green =					{0,  167,0,		255};
const rgb_color LightGreen =			{90, 240,90,	255};
const rgb_color Blue =					{49, 61, 225,	255};
const rgb_color LightBlue =				{64, 162,255,	255};
const rgb_color Purple =				{144,64, 221,	255};
const rgb_color LightPurple =			{166,74, 255,	255};
const rgb_color Lavender =				{193,122,255,	255};
const rgb_color Yellow =				{255,203,0,		255};
const rgb_color Orange =				{255,163,0,		255};
const rgb_color Flesh =					{255,231,186,	255};
const rgb_color Tan =					{208,182,121,	255};
const rgb_color Brown =					{154,110,45,	255};
const rgb_color Grey = 					{200,200,200,	255};
const rgb_color LightMetallicBlue =		{143,166,240,	255};
const rgb_color MedMetallicBlue =		{75, 96, 154,	255};
const rgb_color DarkMetallicBlue =		{78, 89, 126,	255};

//XSheet colors
const rgb_color MedXSheetActiveColor =				{187, 166, 170,	255};
const rgb_color LightXSheetActiveColor =			{239, 227, 218,	255};
const rgb_color DarkXSheetActiveColor =				{143, 109, 120,	255};

const rgb_color MedXSheetSelectedColor =			{178,178,182,	255};
const rgb_color LightXSheetSelectedColor =			{230, 230, 236,	255};
const rgb_color DarkXSheetSelectedColor =			{133,133,135,	255};

const rgb_color MedXSheetSelectedActiveColor =		{143, 109, 120,	255};
const rgb_color LightXSheetSelectedActiveColor =	{214, 191, 192,	255};
const rgb_color DarkXSheetSelectedActiveColor =		{116,88,97,		255};

//const rgb_color DarkXSheetSelectedActiveColor =		{78	,58	,64,	255};

#endif // COLORS_H
