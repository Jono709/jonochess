/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
 *                                                                                                                            *
 *	          JONOCHESS                                                                                                       *
 *                                                                                                                            *
 *	                     copyright* jono    2021                                                                              *
 *                                                                                                                            *
 *	                                                                               kekW LICENSE v6.9                          *
 *                                                                                 <https://unlicense.org/>                   *
 *	                                                                                                                          *
 *                                                                                                                            *
 *	                                                                                                                          *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

#define GLEW_STATIC
#define MAX_OBJ_BUFFER_MACRO 8192 /* if your .obj file has more than 300 triangles, this is too little to handle it!!! */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* strcmp() */
#include <windows.h> /* timeGetTime() */


struct textureBuffer_s{

	const GLchar* name;
	GLuint id;
	GLint width;
	GLint height;
	GLubyte* pixArr = NULL;
};

struct objectBuffer_s {

	float* mBuffer = NULL;
	float* vBuffer = NULL;
	float* tBuffer = NULL;
	float* nBuffer = NULL;
	size_t sizeOfBuffer = MAX_OBJ_BUFFER_MACRO;
	GLuint noIndices = 0; /* assume 0 if not updated */
	GLuint VBO = 0;
	GLuint VAO = 0;
};

struct shaderBuffer_s{

	GLuint program;
	GLint vertexShader;
	GLint fragmentShader;
	char* vShaderBuffer = NULL;
	char* fShaderBuffer = NULL;
};

struct mousePosition_s{

	double x;
	double y;
	double offsetX = 0.0f; /* assume 0 if not updated */
	double offsetY = 0.0f;
};

struct windowResolution_s {

	long x = 720;
	long y = 720;
};

struct windowsTimer_s{

	int time;
	unsigned int ms;
	int start;
	int end;
};

struct uniformMat4_s {

	glm::mat4 mat4 = glm::mat4(1.0f); /* make sure it's initialized to identity matrix first */
	GLint location;
	const char* name;
};

struct bitmapInfoHeader_s{

	int width, height;
	unsigned long junk;
	unsigned int compressionMode;
	unsigned int imageSize;
} ;

struct pngInfoHeader_s {

	int width, height;
};

struct kingPiece_s {

	char currentIndex;
	bool canCastle = true;
	bool canCastleLong = true;
	bool canCastleShort = true;
	bool isInCheck = false;
	bool wantsToMove = false;
};

enum chessPiece_e {

	NONE,
	WHITE_PAWN,
	WHITE_KNIGHT,
	WHITE_BISHOP,
	WHITE_ROOK,
	WHITE_QUEEN,
	WHITE_KING,
	BLACK_PAWN,
	BLACK_KNIGHT,
	BLACK_BISHOP,
	BLACK_ROOK,
	BLACK_QUEEN,
	BLACK_KING
};

enum errorCode_e {

	COMPILE_VS, COMPILE_FS, LINK_PROGRAM,
	FIRST_TEMP_BUFFER_ALLOCATION, SECOND_TEMP_BUFFER_ALLOCATION, THIRD_TEMP_BUFFER_ALLOCATION, MAIN_BUFFER_ALLOCATION,
	BEFORE_REALLOCATION, DURING_REALLOCATION
};


/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
 *                                                                                                                            *
 *           Global Variables                                                                                                 *
 *	                                                                                                                          *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

struct currentlyMovingPiece_s { /* As Mouse Input Is Handled By Callbacks And I Can't Pass This In There */

	float xShift = 0.0f;
	float yShift = 0.0f;
	char indexOrg;
	chessPiece_e piece = NONE; /* Remember To Reset This After Move */
	bool exists = false;
}movingPiece;

struct chessState_s {

	unsigned int piece;
} currentState[8][8] = {

	{ BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK },
	{ BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,  BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN },
	{ NONE,       NONE,         NONE,         NONE,        NONE,       NONE,         NONE,         NONE       },
	{ NONE,       NONE,         NONE,         NONE,        NONE,       NONE,         NONE,         NONE       },
	{ NONE,       NONE,         NONE,         NONE,        NONE,       NONE,         NONE,         NONE       },
	{ NONE,       NONE,         NONE,         NONE,        NONE,       NONE,         NONE,         NONE       },
	{ WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,  WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN },
	{ WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK }
};

struct chessCoord_s {

	char coord;
} currentCoord[8][8] = {

	{0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18},
	{0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28},
	{0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38},
	{0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48},
	{0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58},
	{0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68},
	{0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78},
	{0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88}
};

mousePosition_s mousePosition = {};
windowResolution_s windowResolution = {};
kingPiece_s whiteKing = {}; /* Used To Check For Checks & Castling */
kingPiece_s blackKing = {};
chessState_s temporaryState[8][8];
bool bScreenIsFlipped = false;
bool bCurrentTurn = false; /* False For White, True For Black */
bool bAlreadyChecking = false; /* Avoid Infinite Loops */
bool bWantsToPromote = false;
bool bValidPromotion = false;
bool bHandleEnPassant = false;
int iCanEnPassant = 0;
int iEnPassantLetter = 0;


 /* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
  *                                                                                                                            *
  *           SLOW OBJ 0.1a                                                                                                    *
  *	                                                                                                                           *
  * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

#define LINE_BUFFER_MACRO 64 /* jono, I have no idea what the max strlen in .obj is, but this seems to be enough */


void objectLoaderError(objectBuffer_s* obo, errorCode_e errorCode) {

	switch ( errorCode ) {
	case( FIRST_TEMP_BUFFER_ALLOCATION ):
		fputs("Memory error", stderr);
		break;
	case( SECOND_TEMP_BUFFER_ALLOCATION ):
		free(obo->vBuffer); obo->vBuffer = NULL;
		fputs("Memory error", stderr);
		break;
	case( THIRD_TEMP_BUFFER_ALLOCATION ):
		free(obo->vBuffer); obo->vBuffer = NULL; free(obo->tBuffer); obo->tBuffer = NULL;
		fputs("Memory error", stderr);
		break;
	case( MAIN_BUFFER_ALLOCATION ):
		free(obo->vBuffer); obo->vBuffer = NULL; free(obo->tBuffer); obo->tBuffer = NULL;
		free(obo->nBuffer); obo->nBuffer = NULL;
		fputs("Memory error", stderr);
		break;
	case( BEFORE_REALLOCATION ):
		free(obo->vBuffer); obo->vBuffer = NULL; free(obo->tBuffer); obo->tBuffer = NULL;
		free(obo->nBuffer); obo->nBuffer = NULL; free(obo->mBuffer); obo->mBuffer = NULL;
		fputs("Memory error, Possibly Too Many Triangles In .obj", stderr);
		break;
	case( DURING_REALLOCATION ):
		free(obo->mBuffer); obo->mBuffer = NULL;
		fputs("Memory error", stderr);
		break;
	}
}

int parseOBJ(FILE* FHDL, objectBuffer_s* obo) {

	char str[LINE_BUFFER_MACRO];

	unsigned int cursorv = 0;
	unsigned int cursort = 0;
	unsigned int cursorn = 0;
	unsigned int c = 0;

	char id[4]; id[3] = '\0';
	float vx, vy, vz;
	float tx, ty;
	float nx, ny, nz;
	char* end;
	unsigned int fv0, ft0, fn0, fv1, ft1, fn1, fv2, ft2, fn2;


	/* Get Next Line Until EOF */
	while (fgets(str, LINE_BUFFER_MACRO, FHDL) != NULL) {

		/* Get Line ID */
		sscanf_s(str, "%s", &id, 4);

		if (!strcmp(id, "v")) {
			str[0] = ' ';
			vx = strtof(str, &end); obo->vBuffer[cursorv] = vx; cursorv++;
			vy = strtof(end, &end); obo->vBuffer[cursorv] = vy; cursorv++;
			vz = strtof(end, NULL); obo->vBuffer[cursorv] = vz; cursorv++;

		} else if (!strcmp(id, "vt")) {
			str[0] = ' '; str[1] = ' ';
			tx = strtof(str, &end); obo->tBuffer[cursort] = tx; cursort++;
			ty = strtof(end, NULL); obo->tBuffer[cursort] = ty; cursort++;

		} else if (!strcmp(id, "vn")) {
			str[0] = ' '; str[1] = ' ';
			nx = strtof(str, &end); obo->nBuffer[cursorn] = nx; cursorn++;
			ny = strtof(end, &end); obo->nBuffer[cursorn] = ny; cursorn++;
			nz = strtof(end, NULL); obo->nBuffer[cursorn] = nz; cursorn++;

		} else if (!strcmp(id, "f")) {
			str[0] = ' ';
			sscanf_s(str, "%u/%u/%u %u/%u/%u %u/%u/%u", &fv0, &ft0, &fn0, &fv1, &ft1, &fn1, &fv2, &ft2, &fn2);

			/* Convert Line# to Index# */
			fv0 -= 1; ft0 -= 1; fn0 -= 1; fv1 -= 1; ft1 -= 1; fn1 -= 1; fv2 -= 1; ft2 -= 1; fn2 -= 1;
			fv0 *= 3; ft0 *= 2; fn0 *= 3; fv1 *= 3; ft1 *= 2; fn1 *= 3; fv2 *= 3; ft2 *= 2; fn2 *= 3;

			/* Format Main Buffer */
			obo->mBuffer[c] = obo->vBuffer[fv0]; c++; fv0++;
			obo->mBuffer[c] = obo->vBuffer[fv0]; c++; fv0++;
			obo->mBuffer[c] = obo->vBuffer[fv0]; c++;
			obo->mBuffer[c] = obo->tBuffer[ft0]; c++; ft0++;
			obo->mBuffer[c] = obo->tBuffer[ft0]; c++;
			obo->mBuffer[c] = obo->nBuffer[fn0]; c++; fn0++;
			obo->mBuffer[c] = obo->nBuffer[fn0]; c++; fn0++;
			obo->mBuffer[c] = obo->nBuffer[fn0]; c++;

			obo->mBuffer[c] = obo->vBuffer[fv1]; c++; fv1++;
			obo->mBuffer[c] = obo->vBuffer[fv1]; c++; fv1++;
			obo->mBuffer[c] = obo->vBuffer[fv1]; c++;
			obo->mBuffer[c] = obo->tBuffer[ft1]; c++; ft1++;
			obo->mBuffer[c] = obo->tBuffer[ft1]; c++;
			obo->mBuffer[c] = obo->nBuffer[fn1]; c++; fn1++;
			obo->mBuffer[c] = obo->nBuffer[fn1]; c++; fn1++;
			obo->mBuffer[c] = obo->nBuffer[fn1]; c++;

			obo->mBuffer[c] = obo->vBuffer[fv2]; c++; fv2++;
			obo->mBuffer[c] = obo->vBuffer[fv2]; c++; fv2++;
			obo->mBuffer[c] = obo->vBuffer[fv2]; c++;
			obo->mBuffer[c] = obo->tBuffer[ft2]; c++; ft2++;
			obo->mBuffer[c] = obo->tBuffer[ft2]; c++;
			obo->mBuffer[c] = obo->nBuffer[fn2]; c++; fn2++;
			obo->mBuffer[c] = obo->nBuffer[fn2]; c++; fn2++;
			obo->mBuffer[c] = obo->nBuffer[fn2]; c++;

			/* Keep Track of How Many Indices To Draw */
			obo->noIndices += 3;
		}
	}


	return c;
}

void readOBJ(const char* filepath, objectBuffer_s* obo) {

	FILE* FHDL;
	

	fopen_s(&FHDL, filepath, "r");
	if (FHDL == NULL) { fputs("File error", stderr); exit(1); }

	obo->sizeOfBuffer = parseOBJ(FHDL, obo);

	fclose(FHDL);
}

void loadOBJ(const char* filepath, objectBuffer_s* obo) {

	float* safetypointer = NULL;


	/* Allocate Buffers */
	obo->vBuffer = (float*)malloc(obo->sizeOfBuffer * sizeof(float));
	if (obo->vBuffer == NULL) { objectLoaderError(obo,  FIRST_TEMP_BUFFER_ALLOCATION);  exit(2); }

	obo->tBuffer = (float*)malloc(obo->sizeOfBuffer * sizeof(float));
	if (obo->tBuffer == NULL) { objectLoaderError(obo,  SECOND_TEMP_BUFFER_ALLOCATION); exit(2); }

	obo->nBuffer = (float*)malloc(obo->sizeOfBuffer * sizeof(float));
	if (obo->nBuffer == NULL) { objectLoaderError(obo,  THIRD_TEMP_BUFFER_ALLOCATION);  exit(2); }

	obo->mBuffer = (float*)malloc(obo->sizeOfBuffer * sizeof(float));
	if (obo->mBuffer == NULL) { objectLoaderError(obo,  MAIN_BUFFER_ALLOCATION);        exit(2); }

	/* Populate Buffers */
	readOBJ(filepath, obo);
	if (obo->sizeOfBuffer > MAX_OBJ_BUFFER_MACRO) { objectLoaderError(obo,  BEFORE_REALLOCATION); exit(2); }

	/* Free Temp Buffers */
	free(obo->vBuffer); obo->vBuffer = NULL;
	free(obo->tBuffer); obo->tBuffer = NULL;
	free(obo->nBuffer); obo->nBuffer = NULL;
	
	/* Resize Main Buffer */
	safetypointer = (float*)realloc(obo->mBuffer, ((obo->sizeOfBuffer) * sizeof(float)));
	if (safetypointer == NULL) { objectLoaderError(obo,  DURING_REALLOCATION); exit(2); }
	obo->mBuffer = safetypointer;
}

void InitObject(const char* filepath, objectBuffer_s* obo) {

	loadOBJ(filepath, obo);

	/* Load Main Buffer To VRAM */
	glGenVertexArrays(1, &obo->VAO);
	glGenBuffers(1, &obo->VBO);

	glBindBuffer(GL_ARRAY_BUFFER, obo->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obo->sizeOfBuffer, obo->mBuffer, GL_STATIC_DRAW);

	/* Debugging */
	/* for (int i = 0; i < obo.sizeOfBuffer; i++) { printf("%f\n", obo.mBuffer[i]);} */

	/* Deallocate Main Buffer From RAM */
	free(obo->mBuffer); obo->mBuffer = NULL;

	/* Tell Shader How The Buffer Is Formatted */
	glBindVertexArray(obo->VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}


/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
 *                                                                                                                            *
 *            Shader Initializer                                                                                              *
 *	                                                                                                                          *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

void getErrorLog(shaderBuffer_s* sbo, errorCode_e errorCode) {
	
	GLint logSize = 0;
	char* errorLog = NULL;


	switch ( errorCode ) {
	case( COMPILE_VS ):
		glGetShaderiv(sbo->vertexShader, GL_INFO_LOG_LENGTH, &logSize);
		break;
	case( COMPILE_FS ):
		glGetShaderiv(sbo->fragmentShader, GL_INFO_LOG_LENGTH, &logSize);
		break;
	case( LINK_PROGRAM ):
		glGetProgramiv(sbo->program, GL_INFO_LOG_LENGTH, &logSize);
		break;
	}
	
	errorLog = (char*)malloc(logSize * sizeof(char));
	if (errorLog == NULL) printf("errorLog failed on init ");
	
	switch ( errorCode ) {
	case( COMPILE_VS ):
		glGetShaderInfoLog(sbo->vertexShader, logSize, NULL, errorLog);
		break;
	case( COMPILE_FS ):
		glGetShaderInfoLog(sbo->fragmentShader, logSize, NULL, errorLog);
		break;
	case( LINK_PROGRAM ):
		glGetProgramInfoLog(sbo->program, logSize, NULL, errorLog);
		break;
	}

	printf("%s", errorLog);

	free(errorLog); errorLog = NULL;
}

void compileShader(shaderBuffer_s* sbo) {

	GLint iscompiled = 0;
	

	/* Vertex Shader*/
	sbo->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(sbo->vertexShader, 1, &sbo->vShaderBuffer, NULL);
	glCompileShader(sbo->vertexShader);

	glGetShaderiv(sbo->vertexShader, GL_COMPILE_STATUS, &iscompiled);
	if (iscompiled == GL_FALSE) getErrorLog(sbo,  COMPILE_VS);

	/* Fragment Shader*/
	sbo->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(sbo->fragmentShader, 1, &sbo->fShaderBuffer, NULL);
	glCompileShader(sbo->fragmentShader);

	glGetShaderiv(sbo->fragmentShader, GL_COMPILE_STATUS, &iscompiled);
	if (iscompiled == GL_FALSE) getErrorLog(sbo,  COMPILE_FS);
}

void linkShader(shaderBuffer_s* sbo) {
	
	GLint islinked = 0;
	

	sbo->program = glCreateProgram();

	glAttachShader(sbo->program, sbo->vertexShader);
	glAttachShader(sbo->program, sbo->fragmentShader);
	glLinkProgram(sbo->program);

	glGetProgramiv(sbo->program, GL_LINK_STATUS, &islinked);
	if (islinked == GL_FALSE) { getErrorLog(sbo,  LINK_PROGRAM); glDeleteProgram(sbo->program); }
}

void readShaderFile(const char* vertexPath, const char* fragmentPath, shaderBuffer_s* sbo) {

	FILE* vsFHDL;
	FILE* fsFHDL;
	
	size_t VSresult = 0;
	size_t FSresult = 0;

	long VSsize = 0;
	long FSsize = 0;


	/* Open Files */
	fopen_s(&vsFHDL, vertexPath, "rb");                                                  // that b makes all the difference
	if (vsFHDL == NULL) { fputs("File error", stderr); exit(1); }

	fopen_s(&fsFHDL, fragmentPath, "rb");
	if (fsFHDL == NULL) { fputs("File error", stderr); exit(1); }

	/* Get File Sizes */
	fseek(vsFHDL, 0, SEEK_END);
	VSsize = ftell(vsFHDL);
	rewind(vsFHDL);

	fseek(fsFHDL, 0, SEEK_END);
	FSsize = ftell(fsFHDL);
	rewind(fsFHDL);

	/* Allocate Enough Memory For Buffers */
	sbo->vShaderBuffer = (char*)malloc((sizeof(char) * VSsize) + 1);
	if (sbo->vShaderBuffer == NULL) { fputs("Memory error", stderr); exit(2); }

	sbo->fShaderBuffer = (char*)malloc((sizeof(char) * FSsize) + 1);
	if (sbo->fShaderBuffer == NULL) { fputs("Memory error", stderr); exit(2); }
	
	/* Read Files To Buffers And Ensure Trailing Zero */
	VSresult = fread(sbo->vShaderBuffer, 1, VSsize, vsFHDL);
	if (VSresult != VSsize) { fputs("Reading error", stderr); exit(3); }
	sbo->vShaderBuffer[VSsize] = '\0';

	FSresult = fread(sbo->fShaderBuffer, 1, FSsize, fsFHDL);
	if (FSresult != FSsize) { fputs("Reading error", stderr); exit(3); }
	sbo->fShaderBuffer[FSsize] = '\0';

	/* Done */
	fclose(vsFHDL);
	fclose(fsFHDL);
}

void InitShader(const char* vertexPath, const char* fragmentPath, shaderBuffer_s* sbo) {

	readShaderFile(vertexPath, fragmentPath, sbo);
	compileShader(sbo);
	linkShader(sbo);

	/* Cleanup */
	glDetachShader(sbo->program, sbo->vertexShader);
	glDetachShader(sbo->program, sbo->fragmentShader);

	glDeleteShader(sbo->vertexShader);
	glDeleteShader(sbo->fragmentShader);

	free(sbo->vShaderBuffer); sbo->vShaderBuffer = NULL;
	free(sbo->fShaderBuffer); sbo->fShaderBuffer = NULL;
}

void shaderUse(GLint program) {
	
	glUseProgram(program);
}

void setTextureUniform(shaderBuffer_s* so, textureBuffer_s* tbo ) {

	glUniform1i(glGetUniformLocation(so->program, tbo->name), 0);
}


/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
 *                                                                                                                            *
 *            Texture Initializer                                                                                             *
 *	                                                                                                                          *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

void loadBMP(const char* filepath, textureBuffer_s* tbo) {

	FILE* FHDL;
	size_t result = 0;
	unsigned short bmMagic;
	bitmapInfoHeader_s infoheader;
	

	fopen_s(&FHDL, filepath, "rb");
	if (FHDL == NULL) { fputs("File error", stderr); exit(1); }

	result = fread(&bmMagic, 1, 2, FHDL);
	if (result != 2) { fputs("Reading error", stderr); exit(3); }
	if (bmMagic != 'M' * 256 + 'B') { exit(4444); }

	fseek(FHDL, 18, SEEK_SET);
	result = fread(&infoheader, 1, sizeof(bitmapInfoHeader_s), FHDL);
	if (result != sizeof(bitmapInfoHeader_s)) { fputs("Reading error", stderr); exit(3); }

	tbo->width = infoheader.width;
	tbo->height = infoheader.height;

	tbo->pixArr = (GLubyte*)malloc(infoheader.imageSize);
	if (tbo->pixArr == NULL) { fputs("Memory error", stderr); exit(2); }

	fseek(FHDL, 54, SEEK_SET);
	result = fread(tbo->pixArr, 1, infoheader.imageSize, FHDL);
	if (result != infoheader.imageSize) { fputs("Reading error", stderr); exit(3); }
	
	fclose(FHDL);
}

void loadPNG(const char* filepath, textureBuffer_s* tbo) {

	FILE* FHDL;
	size_t result = 0;
	unsigned long long pngMagic; // 89 50 4e 47 0d 0a 1a 0a
	pngInfoHeader_s infoheader;
	int width, height, nrChannels;

	fopen_s(&FHDL, filepath, "rb");
	if (FHDL == NULL) { fputs("File error", stderr); exit(1); }

	result = fread(&pngMagic, 1, sizeof(pngMagic), FHDL);
	if (result != sizeof(pngMagic)) { fputs("Reading error", stderr); exit(3); }
	pngMagic = _byteswap_uint64(pngMagic);

	fseek(FHDL, 16, SEEK_SET);
	result = fread(&infoheader, 1, sizeof(pngInfoHeader_s), FHDL);
	if (result != sizeof(pngInfoHeader_s)) { fputs("Reading error", stderr); exit(3); }

	unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 0);
	
	size_t sizepix = width * height * nrChannels;
	tbo->pixArr = (GLubyte*)malloc(sizepix); 
	if (tbo->pixArr == NULL) { fputs("Memory error", stderr); exit(2); }

	tbo->pixArr = data;
	/* &tbo->pixArr; */
	/* &data; */
	tbo->width = width;
	tbo->height = height;

	fclose(FHDL);
}

void InitTexture(const char* filepath, textureBuffer_s* tbo) {

	glGenTextures(1, &tbo->id);
	glBindTexture(GL_TEXTURE_2D, tbo->id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	loadPNG(filepath, tbo);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tbo->width, tbo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tbo->pixArr);
	glGenerateMipmap(GL_TEXTURE_2D);

	free(tbo->pixArr);
}


/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
 *                                                                                                                            *
 *            Drawing Functions                                                                                               *
 *	                                                                                                                          *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

void setGridUniform(float x, float y, uniformMat4_s* uniform) {
	
	uniform->mat4 = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	if (bScreenIsFlipped) {
		uniform->mat4 = glm::rotate(uniform->mat4, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	uniform->mat4 = glm::translate(uniform->mat4, glm::vec3(x, y, 0.0f));
	glUniformMatrix4fv(uniform->location, 1, GL_FALSE, glm::value_ptr(uniform->mat4));
}

void drawTris(objectBuffer_s* obo) {

	glBindVertexArray(obo->VAO);
	glDrawArrays(GL_TRIANGLES, 0, obo->noIndices);
}

/* jono, Might Be A Little Bit Cleaning Up Here To Do :) */
void drawCall(int i, int j, uniformMat4_s* uniform, objectBuffer_s* obo) {
	
	switch (i) {
	case(0):
		if      (j == 0) { setGridUniform(0.0f,   0.0f,  uniform); drawTris(obo); }
		else if (j == 1) { setGridUniform(0.25f,  0.0f,  uniform); drawTris(obo); }
		else if (j == 2) { setGridUniform(0.5f,   0.0f,  uniform); drawTris(obo); }
		else if (j == 3) { setGridUniform(0.75f,  0.0f,  uniform); drawTris(obo); }
		else if (j == 4) { setGridUniform(1.0f,   0.0f,  uniform); drawTris(obo); }
		else if (j == 5) { setGridUniform(1.25f,  0.0f,  uniform); drawTris(obo); }
		else if (j == 6) { setGridUniform(1.5f,   0.0f,  uniform); drawTris(obo); }
		else /*  j == 7*/{ setGridUniform(1.75f,  0.0f,  uniform); drawTris(obo); }
		break;
	case(1):
		if      (j == 0) { setGridUniform(0.0f,  -0.25f, uniform); drawTris(obo); }
		else if (j == 1) { setGridUniform(0.25f, -0.25f, uniform); drawTris(obo); }
		else if (j == 2) { setGridUniform(0.5f,  -0.25f, uniform); drawTris(obo); }
		else if (j == 3) { setGridUniform(0.75f, -0.25f, uniform); drawTris(obo); }
		else if (j == 4) { setGridUniform(1.0f,  -0.25f, uniform); drawTris(obo); }
		else if (j == 5) { setGridUniform(1.25f, -0.25f, uniform); drawTris(obo); }
		else if (j == 6) { setGridUniform(1.5f,  -0.25f, uniform); drawTris(obo); }
		else /*  j == 7*/{ setGridUniform(1.75f, -0.25f, uniform); drawTris(obo); }
		break;
	case(2):
		if      (j == 0) { setGridUniform(0.0f,  -0.5f,  uniform); drawTris(obo); }
		else if (j == 1) { setGridUniform(0.25f, -0.5f,  uniform); drawTris(obo); }
		else if (j == 2) { setGridUniform(0.5f,  -0.5f,  uniform); drawTris(obo); }
		else if (j == 3) { setGridUniform(0.75f, -0.5f,  uniform); drawTris(obo); }
		else if (j == 4) { setGridUniform(1.0f,  -0.5f,  uniform); drawTris(obo); }
		else if (j == 5) { setGridUniform(1.25f, -0.5f,  uniform); drawTris(obo); }
		else if (j == 6) { setGridUniform(1.5f,  -0.5f,  uniform); drawTris(obo); }
		else /*  j == 7*/{ setGridUniform(1.75f, -0.5f,  uniform); drawTris(obo); }
		break;
	case(3):
		if      (j == 0) { setGridUniform(0.0f,  -0.75f, uniform); drawTris(obo); }
		else if (j == 1) { setGridUniform(0.25f, -0.75f, uniform); drawTris(obo); }
		else if (j == 2) { setGridUniform(0.5f,  -0.75f, uniform); drawTris(obo); }
		else if (j == 3) { setGridUniform(0.75f, -0.75f, uniform); drawTris(obo); }
		else if (j == 4) { setGridUniform(1.0f,  -0.75f, uniform); drawTris(obo); }
		else if (j == 5) { setGridUniform(1.25f, -0.75f, uniform); drawTris(obo); }
		else if (j == 6) { setGridUniform(1.5f,  -0.75f, uniform); drawTris(obo); }
		else /*  j == 7*/{ setGridUniform(1.75f, -0.75f, uniform); drawTris(obo); }
		break;
	case(4):
		if      (j == 0) { setGridUniform(0.0f,  -1.0f,  uniform); drawTris(obo); }
		else if (j == 1) { setGridUniform(0.25f, -1.0f,  uniform); drawTris(obo); }
		else if (j == 2) { setGridUniform(0.5f,  -1.0f,  uniform); drawTris(obo); }
		else if (j == 3) { setGridUniform(0.75f, -1.0f,  uniform); drawTris(obo); }
		else if (j == 4) { setGridUniform(1.0f,  -1.0f,  uniform); drawTris(obo); }
		else if (j == 5) { setGridUniform(1.25f, -1.0f,  uniform); drawTris(obo); }
		else if (j == 6) { setGridUniform(1.5f,  -1.0f,  uniform); drawTris(obo); }
		else /*  j == 7*/{ setGridUniform(1.75f, -1.0f,  uniform); drawTris(obo); }
		break;
	case(5):
		if      (j == 0) { setGridUniform(0.0f,  -1.25f, uniform); drawTris(obo); }
		else if (j == 1) { setGridUniform(0.25f, -1.25f, uniform); drawTris(obo); }
		else if (j == 2) { setGridUniform(0.5f,  -1.25f, uniform); drawTris(obo); }
		else if (j == 3) { setGridUniform(0.75f, -1.25f, uniform); drawTris(obo); }
		else if (j == 4) { setGridUniform(1.0f,  -1.25f, uniform); drawTris(obo); }
		else if (j == 5) { setGridUniform(1.25f, -1.25f, uniform); drawTris(obo); }
		else if (j == 6) { setGridUniform(1.5f,  -1.25f, uniform); drawTris(obo); }
		else /*  j == 7*/{ setGridUniform(1.75f, -1.25f, uniform); drawTris(obo); }
		break;
	case(6):
		if      (j == 0) { setGridUniform(0.0f,  -1.5f,  uniform); drawTris(obo); }
		else if (j == 1) { setGridUniform(0.25f, -1.5f,  uniform); drawTris(obo); }
		else if (j == 2) { setGridUniform(0.5f,  -1.5f,  uniform); drawTris(obo); }
		else if (j == 3) { setGridUniform(0.75f, -1.5f,  uniform); drawTris(obo); }
		else if (j == 4) { setGridUniform(1.0f,  -1.5f,  uniform); drawTris(obo); }
		else if (j == 5) { setGridUniform(1.25f, -1.5f,  uniform); drawTris(obo); }
		else if (j == 6) { setGridUniform(1.5f,  -1.5f,  uniform); drawTris(obo); }
		else /*  j == 7*/{ setGridUniform(1.75f, -1.5f,  uniform); drawTris(obo); }
		break;
	case(7):
		if      (j == 0) { setGridUniform(0.0f,  -1.75f, uniform); drawTris(obo); }
		else if (j == 1) { setGridUniform(0.25f, -1.75f, uniform); drawTris(obo); }
		else if (j == 2) { setGridUniform(0.5f,  -1.75f, uniform); drawTris(obo); }
		else if (j == 3) { setGridUniform(0.75f, -1.75f, uniform); drawTris(obo); }
		else if (j == 4) { setGridUniform(1.0f,  -1.75f, uniform); drawTris(obo); }
		else if (j == 5) { setGridUniform(1.25f, -1.75f, uniform); drawTris(obo); }
		else if (j == 6) { setGridUniform(1.5f,  -1.75f, uniform); drawTris(obo); }
		else /*  j == 7*/{ setGridUniform(1.75f, -1.75f, uniform); drawTris(obo); }
		break;
	}
}

void drawChessPieceStatic(uniformMat4_s* uniform, GLint texUniform, objectBuffer_s* obo) {
	
	float xShift = 0.0f; float yShift = 0.0f;

	
	for (int i = 0; i < 8; i++) {

		for (int j = 0; j < 8; j++) {
			
			switch (currentState[i][j].piece) {
			default:
				break;
			case(WHITE_PAWN):
				/*       0.5            -0.25 white pawn	*/
				xShift = 0.5f; yShift = -0.25f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(WHITE_KNIGHT):
				/*       0.0            -0.25 white horsey	*/
				xShift = 0.0f; yShift = -0.25f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(WHITE_BISHOP):
				/*       0.5            -0.75 white bishop	*/
				xShift = 0.5f; yShift = -0.75f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(WHITE_ROOK):
				/*       0.25            -0.25 white rook	*/
				xShift = 0.25f; yShift = -0.25f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(WHITE_QUEEN):
				/*       0.25            -0.75 white queen	*/
				xShift = 0.25f; yShift = -0.75f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(WHITE_KING):
				/*       0.0            -0.75 white king	*/
				xShift = 0.0f; yShift = -0.75f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(BLACK_PAWN):
				/*       0.5            0.0  black pawn  	*/
				xShift = 0.5f; yShift = 0.0f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(BLACK_KNIGHT):
				/*       0.0            0.0  black horsey   */
				xShift = 0.0f; yShift = 0.0f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(BLACK_BISHOP):
				/*       0.5            -0.5  black bishop	*/
				xShift = 0.5f; yShift = -0.5f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(BLACK_ROOK):
				/*       0.25            0.0  black rook	*/
				xShift = 0.25f; yShift = 0.0f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(BLACK_QUEEN):
				/*       0.25            -0.5  black queen	*/
				xShift = 0.25f; yShift = -0.5f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			case(BLACK_KING):
				/*       0.0            -0.5  black king	*/
				xShift = 0.0f; yShift = -0.5f;
				glUniform2f(texUniform, xShift, yShift);
				drawCall(i, j, uniform, obo);
				break;
			}
		}
	}
}

void drawChessPieceDynamic(uniformMat4_s* uniform, GLint texUniform, objectBuffer_s* obo) {

	uniform->mat4 = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	uniform->mat4 = glm::translate(uniform->mat4, glm::vec3(mousePosition.x, -mousePosition.y, 0.0f));
	glUniformMatrix4fv(uniform->location, 1, GL_FALSE, glm::value_ptr(uniform->mat4));
	glUniform2f(texUniform, movingPiece.xShift, movingPiece.yShift);
	glBindVertexArray(obo->VAO);
	glDrawArrays(GL_TRIANGLES, 0, obo->noIndices);
}

void freeBufferObject(objectBuffer_s* obo) {

	glDeleteVertexArrays(1, &obo->VAO);
	glDeleteBuffers(1, &obo->VBO);
}


/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
 *                                                                                                                            *
 *            Game Logic                                                                                                      *
 *	                                                                                                                          *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

int validatePawnMove(char indexWish, char indexOrg);
int validateKnightMove(char indexWish, char indexOrg);
int validateBishopMove(char indexWish, char indexOrg);
int validateRookMove(char indexWish, char indexOrg);
int validateQueenMove(char indexWish, char indexOrg);
int validateKingMove(char indexWish, char indexOrg);


/* More Cleaning Up */
char getChessIndex(double posx, double posy) { /* In Reference To chessState[num][let]*/
	
	char index = 0x00;
	
	
	/* jono, TODO: Think About A Better Way To Get This, Maybe With Collision Detection Algorithms */
	if (bScreenIsFlipped) {
		if (posx > -1.0f && posx < -0.75f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x18; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x28; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x38; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x48; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x58; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x68; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x78; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x88; }
		}
		else if (posx > -0.75f && posx < -0.5f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x17; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x27; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x37; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x47; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x57; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x67; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x77; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x87; }
		}
		else if (posx > -0.5f && posx < -0.25f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x16; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x26; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x36; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x46; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x56; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x66; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x76; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x86; }
		}
		else if (posx > -0.25f && posx < 0.0f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x15; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x25; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x35; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x45; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x55; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x65; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x75; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x85; }
		}
		else if (posx > 0.0f && posx < 0.25f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x14; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x24; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x34; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x44; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x54; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x64; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x74; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x84; }
		}
		else if (posx > 0.25f && posx < 0.5f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x13; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x23; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x33; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x43; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x53; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x63; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x73; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x83; }
		}
		else if (posx > 0.5f && posx < 0.75f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x12;}
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x22; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x32; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x42; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x52; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x62; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x72; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x82; }
		}
		else if (posx > 0.75f && posx < 1.0f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x11; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x21; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x31; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x41; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x51; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x61; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x71; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x81; }
		}
	}
	else {
		if (posx > -1.0f && posx < -0.75f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x81; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x71; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x61; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x51; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x41; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x31; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x21; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x11; }
		}
		else if (posx > -0.75f && posx < -0.5f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x82; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x72; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x62; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x52; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x42; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x32; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x22; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x12; }
		}
		else if (posx > -0.5f && posx < -0.25f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x83; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x73; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x63; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x53; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x43; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x33; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x23; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x13; }
		}
		else if (posx > -0.25f && posx < 0.0f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x84; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x74; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x64; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x54; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x44; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x34; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x24; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x14; }
		}
		else if (posx > 0.0f && posx < 0.25f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x85; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x75; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x65; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x55; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x45; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x35; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x25; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x15; }
		}
		else if (posx > 0.25f && posx < 0.5f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x86; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x76; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x66; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x56; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x46; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x36; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x26; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x16; }
		}
		else if (posx > 0.5f && posx < 0.75f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x87; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x77; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x67; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x57; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x47; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x37; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x27; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x17; }
		}
		else if (posx > 0.75f && posx < 1.0f) {
			if (posy > 0.75f && posy < 1.0f)        { index = 0x88; }
			else if (posy > 0.5f && posy < 0.75f)   { index = 0x78; }
			else if (posy > 0.25f && posy < 0.5f)   { index = 0x68; }
			else if (posy > 0.0f && posy < 0.25f)   { index = 0x58; }
			else if (posy > -0.25f && posy < 0.0f)  { index = 0x48; }
			else if (posy > -0.5f && posy < -0.25f) { index = 0x38; }
			else if (posy > -0.75f && posy < -0.5f) { index = 0x28; }
			else if (posy > -1.0f && posy < -0.75f) { index = 0x18; }
		}
	}
	return index;
}

bool isBlack(unsigned int state, chessPiece_e* piece) { /* Returns True For Error */

	switch (state) {
	default:
		return true;
	case(BLACK_PAWN):
		*piece = BLACK_PAWN;
		movingPiece.xShift = 0.5f; movingPiece.yShift = 0.0f;
		return false;
	case(BLACK_KNIGHT):
		*piece = BLACK_KNIGHT;
		movingPiece.xShift = 0.0f; movingPiece.yShift = 0.0f;
		return false;
	case(BLACK_BISHOP):
		*piece = BLACK_BISHOP;
		movingPiece.xShift = 0.5f; movingPiece.yShift = -0.5f;
		return false;
	case(BLACK_ROOK):
		*piece = BLACK_ROOK;
		movingPiece.xShift = 0.25f; movingPiece.yShift = 0.0f;
		return false;
	case(BLACK_QUEEN):
		*piece = BLACK_QUEEN;
		movingPiece.xShift = 0.25f; movingPiece.yShift = -0.5f;
		return false;
	case(BLACK_KING):
		*piece = BLACK_KING;
		movingPiece.xShift = 0.0f; movingPiece.yShift = -0.5f;
		return false;
	}

	/* Never Gets Here */
	return true;
}

bool isWhite(unsigned int state, chessPiece_e* piece) { /* Returns True For Error */

	switch (state) {
	default:
		return true;
	case(WHITE_PAWN):
		*piece = WHITE_PAWN;
		movingPiece.xShift = 0.5f; movingPiece.yShift = -0.25f;
		return false;
	case(WHITE_KNIGHT):
		*piece = WHITE_KNIGHT;
		movingPiece.xShift = 0.0f; movingPiece.yShift = -0.25f;
		return false;
	case(WHITE_BISHOP):
		*piece = WHITE_BISHOP;
		movingPiece.xShift = 0.5f; movingPiece.yShift = -0.75f;
		return false;
	case(WHITE_ROOK):
		*piece = WHITE_ROOK;
		movingPiece.xShift = 0.25f; movingPiece.yShift = -0.25f;
		return false;
	case(WHITE_QUEEN):
		*piece = WHITE_QUEEN;
		movingPiece.xShift = 0.25f; movingPiece.yShift = -0.75f;
		return false;
	case(WHITE_KING):
		*piece = WHITE_KING;
		movingPiece.xShift = 0.0f; movingPiece.yShift = -0.75f;
		return false;
	}

	/* Never Gets Here */
	return true;
}

int checkValidTurn(char index, chessPiece_e* piece) { /* Returns 1 For Error */
	
	char v = ((index & 0xf0) >> 4) - 1;
	char u = (index & 0x0f) - 1;
	

	if (bCurrentTurn && !isBlack(currentState[v][u].piece, piece)) { /* black turn and org piece is black */
		currentState[v][u].piece = NONE;
		return 0;
	}
	if (!bCurrentTurn && !isWhite(currentState[v][u].piece, piece)) { /* white turn and org piece is white */
		currentState[v][u].piece = NONE;
		return 0;
	}
	
	return 1;
}

void startPlayerMove(void) {

	char indexOrg = 0x00;
	double posx = mousePosition.x;
	double posy = mousePosition.y;
	chessPiece_e chessPiece = NONE;
	/* printf("x:%f y:%f\n", posx, posy); */

	indexOrg = getChessIndex(posx, posy);
	movingPiece.indexOrg = indexOrg;
	if (indexOrg == 0x00) { return; } /* Out Of Bounds */
	if (checkValidTurn(indexOrg, &chessPiece) == 1) { return; }
	movingPiece.piece = chessPiece;
	movingPiece.exists = true;
	
}

void illegalMove(void) { /* Revert State */

	char v = ((movingPiece.indexOrg & 0xf0) >> 4) - 1;
	char u = (movingPiece.indexOrg & 0x0f) - 1;


	currentState[v][u].piece = movingPiece.piece;
}

bool checkForChecks(char kingPos) { /* Actually Checking If There Are ANY Legal Moves On Opponents Upcoming Turn Where Current King Might Be Captured */

	bAlreadyChecking = true;

	if (!bCurrentTurn) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {

				bCurrentTurn = !bCurrentTurn; /* Check From Their Perspective */

				switch (currentState[i][j].piece) {

				case(BLACK_PAWN):
					if (validatePawnMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(BLACK_KNIGHT):
					if (validateKnightMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(BLACK_BISHOP):
					if (validateBishopMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(BLACK_ROOK):
					if (validateRookMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(BLACK_QUEEN):
					if (validateQueenMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(BLACK_KING):
					if (validateKingMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				default:
					break;
				}
				bCurrentTurn = !bCurrentTurn; /* Then Reset */
			}
		}
		return false;
	}
	if (bCurrentTurn) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {

				bCurrentTurn = !bCurrentTurn; /* Check From Their Perspective */

				switch (currentState[i][j].piece) {

				case(WHITE_PAWN):
					if (validatePawnMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(WHITE_KNIGHT):
					if (validateKnightMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(WHITE_BISHOP):
					if (validateBishopMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(WHITE_ROOK):
					if (validateRookMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(WHITE_QUEEN):
					if (validateQueenMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				case(WHITE_KING):
					if (validateKingMove(kingPos, currentCoord[i][j].coord) == 0) { bCurrentTurn = !bCurrentTurn; return true; }
					else { break; }
				default:
					break;
				}
				bCurrentTurn = !bCurrentTurn; /* Then Reset */
			}
		}
		return false;
	}
	return true;
}

int validatePawnMove(char indexWish, char indexOrg) { /* FIXME: Add En Peasant */

	char orgN;
	char wishN;
	char orgL;
	char wishL;
	char alt = 0;
	chessPiece_e ignore = NONE;
	orgN = ((indexOrg & 0xf0) >> 4);
	wishN = ((indexWish & 0xf0) >> 4);
	orgL = (indexOrg & 0x0f);
	wishL = (indexWish & 0x0f);

	if (orgL == wishL) { /* Only Legal If No Piece Infront */
		if (currentState[wishN-1][wishL-1].piece == NONE) {/* nothing */}
		else { return 1; }
	}
	if (!bCurrentTurn) { /* First Moves For Respective Colors */
		if (orgN == 7) {
			alt = -1;
		}
		orgN -= 1; /* Used By The Validation Against WishN */
	}
	if (bCurrentTurn) {
		if (orgN == 2) {
			alt = 1;
		}
		orgN += 1; /* Used By The Validation Against WishN */
	}
	if ((orgL - 1) == wishL || (orgL + 1) == wishL) { /* Eliminate Capturing On Empty Squares */
		if ((!bCurrentTurn && !isBlack(currentState[wishN - 1][wishL - 1].piece, &ignore)) || (bCurrentTurn && !isWhite(currentState[wishN - 1][wishL - 1].piece, &ignore))) {
			/* nothing */
		}
		else if (iCanEnPassant > 0) { /* Unless En Passant Is Available */
			if (wishL == iEnPassantLetter) { bHandleEnPassant = true; }
			else { return 1; }
		}
		else { return 1; }
	}
	if (orgN == wishN)  {
		if (bAlreadyChecking) {
			return 0;
		}
		else {
			if (!bCurrentTurn) {
				if (wishN == 1) {
					bWantsToPromote = true;
					return 0;
				}
				else { return 0; }
			}
			else {
				if (wishN == 8) {
					bWantsToPromote = true;
					return 0;
				}
				else { return 0; }
			}
		}
		
	} 
	if ((orgN += alt) == wishN) {
		if (!bCurrentTurn) {
			if (currentState[wishN - 1][wishL - 1].piece == NONE) {
				{ iCanEnPassant = 2; iEnPassantLetter = wishL; return 0; }
			}
			else { return 1; }
		}
		if (bCurrentTurn) {
			if (currentState[wishN - 1][wishL - 1].piece == NONE) {
				{ iCanEnPassant = 2; iEnPassantLetter = wishL; return 0; }
			}
			else { return 1; }
		}
	}
	return 1;
}

int validateKnightMove(char indexWish, char indexOrg) {

	char orgN;
	char wishN;
	char orgL;
	char wishL;

	orgN = ((indexOrg & 0xf0) >> 4);
	wishN = ((indexWish & 0xf0) >> 4);
	orgL = (indexOrg & 0x0f);
	wishL = (indexWish & 0x0f);

	if ((orgN + 2) == wishN || (orgN - 2) == wishN) {
		if ((orgL + 1) == wishL || (orgL - 1) == wishL) { return 0; }
	}
	if ((orgL + 2) == wishL || (orgL - 2) == wishL) {
		if ((orgN + 1) == wishN || (orgN - 1) == wishN) { return 0; }
	}
	return 1;
}

int validateBishopMove(char indexWish, char indexOrg) {

	char orgN;
	char wishN;
	char orgL;
	char wishL;

	orgN = ((indexOrg & 0xf0) >> 4);
	wishN = ((indexWish & 0xf0) >> 4);
	orgL = (indexOrg & 0x0f);
	wishL = (indexWish & 0x0f);

	if (orgN < wishN) {
		if (orgL < wishL) { /* SE Attempt */
			if ((wishL - orgL) == (wishN - orgN)) {
				char j = wishL - 1;
				for (char i = wishN - 1; i > orgN; i--) {
					if (currentState[i - 1][j - 1].piece == NONE) { j--; }
					else { return 1; }
				}
				return 0;
			}
		}
		if (orgL > wishL) { /* SW Attempt */
			if ((wishL - orgL) == (orgN - wishN)) {
				char j = wishL + 1;
				for (char i = wishN - 1; i > orgN; i--) {
					if (currentState[i - 1][j - 1].piece == NONE) { j++; }
					else { return 1; }
				}
				return 0;
			}
		}
	}
	if (orgN > wishN) {
		if (orgL < wishL) { /* NE Attempt */
			if ((orgN - wishN) == (wishL - orgL)) {
				char j = wishL - 1;
				for (char i = wishN + 1; i < orgN; i++) {
					if (currentState[i - 1][j - 1].piece == NONE) { j--; }
					else { return 1; }
				}
				return 0;
			}
		}
		if (orgL > wishL) { /* NW Attempt */
			if ((wishL - orgL) == (wishN - orgN)) {
				char j = wishL + 1;
				for (char i = wishN + 1; i < orgN; i++) {
					if (currentState[i - 1][j - 1].piece == NONE) { j++; }
					else { return 1; }
				}
				return 0;
			}
		}
	}
	return 1;
}

int validateRookMove(char indexWish, char indexOrg) {

	char orgN;
	char wishN;
	char orgL;
	char wishL;
	bool bValid = false;

	orgN = ((indexOrg & 0xf0) >> 4);
	wishN = ((indexWish & 0xf0) >> 4);
	orgL = (indexOrg & 0x0f);
	wishL = (indexWish & 0x0f);

	if (orgN == wishN) {
		if (orgL < wishL) { /* Moving Right */
			for (char j = wishL -1; j > orgL; j--) {
				if (currentState[wishN - 1][j - 1].piece == NONE) {}
				else { return 1; }
			}
			bValid = true;
		}
		else if (orgL > wishL) { /* Moving Left */
			for (char j = wishL + 1; j < orgL; j++) {
				if (currentState[wishN - 1][j - 1].piece == NONE) {}
				else { return 1; }
			}
			bValid = true;
		}
	} 
	else if (orgL == wishL) {
		if (orgN < wishN) { /* Moving Down */
			for (char i = wishN - 1; i > orgN; i--) {
				if (currentState[i - 1][wishL - 1].piece == NONE) {}
				else { return 1; }
			}
			bValid = true;
		}
		if (orgN > wishN) { /* Moving Up */
			for (char i = wishN + 1; i < orgN; i++) {
				if (currentState[i - 1][wishL - 1].piece == NONE) {}
				else { return 1; }
			}
			bValid = true;
		}
	}
	if (!bCurrentTurn && whiteKing.canCastle) {
		if (orgL == 8 && whiteKing.canCastleShort) {
			if (bValid) {
				whiteKing.canCastleShort = false; /* No Way First Rook Move Puts Own King In Check So Can Update Globals */
				if (whiteKing.canCastleLong) {
					return 0;
				}
				else { whiteKing.canCastle = false; return 0;
				}
			}
		}
		if (orgL == 1 && whiteKing.canCastleLong) {
			if (bValid) {
				whiteKing.canCastleLong = false;
				if (whiteKing.canCastleShort) {
					return 0;
				}
				else {
					whiteKing.canCastle = false; return 0;
				}
			}
		}
	}
	if (bCurrentTurn && blackKing.canCastle) {
		if (orgL == 8 && blackKing.canCastleShort) {
			if (bValid) {
				blackKing.canCastleShort = false;
				if (blackKing.canCastleLong) {
					return 0;
				}
				else {
					blackKing.canCastle = false; return 0;
				}
			}
		}
		if (orgL == 1 && blackKing.canCastleLong) {
			if (bValid) {
				blackKing.canCastleLong = false;
				if (blackKing.canCastleShort) {
					return 0;
				}
				else {
					blackKing.canCastle = false; return 0;
				}
			}
		}
	}
	if (bValid) {
		return 0;
	}
	return 1;
}

int validateQueenMove(char indexWish, char indexOrg) {

	if ((validateBishopMove(indexWish, indexOrg) == 0) || (validateRookMove(indexWish, indexOrg) == 0)) {
		return 0;
	}

	return 1;
}

int validateKingMove(char indexWish, char indexOrg) {

	char orgN;
	char wishN;
	char orgL;
	char wishL;
	bool legalAttempt = false;

	orgN = ((indexOrg & 0xf0) >> 4);
	wishN = ((indexWish & 0xf0) >> 4);
	orgL = (indexOrg & 0x0f);
	wishL = (indexWish & 0x0f);

	if (orgN == wishN) { /* Left Or Right */
		if ((orgL - 1 == wishL) || (orgL + 1 == wishL)) { legalAttempt = true; }
	}
	else if (orgL == wishL) { /* Up Or Down */
		if ((orgN - 1 == wishN) || (orgN + 1 == wishN)) { legalAttempt = true; }
	}
	else if (orgN + 1 == wishN) { /* South Diagonally */
		if ((orgL - 1 == wishL) || (orgL + 1 == wishL)) { legalAttempt = true; }
	}
	else if (orgN - 1 == wishN) { /* North Diagonally */
		if ((orgL - 1 == wishL) || (orgL + 1 == wishL)) { legalAttempt = true; }
	}
	if (bAlreadyChecking) { /* Needs To Stop Here If Came From checkForChecks() */
		if (legalAttempt) {
			return 0;
		}
		else { return 1; }
	}
	if (!bCurrentTurn) {
		if (legalAttempt) {
			
				whiteKing.canCastle = false;
				whiteKing.wantsToMove = true;
				return 0;
		}
		else if (whiteKing.canCastle) {
			if (whiteKing.canCastleShort && (wishL == 7)) {
				
				for (char j = 7; j > 5; j--) {
					if (currentState[7][j - 1].piece == NONE) {}
					else { return 1; }
				}
				if (checkForChecks(whiteKing.currentIndex) || checkForChecks(indexWish) || checkForChecks(0x86)) {
					
					bAlreadyChecking = false;
					return 1;
				}
				else {
					bAlreadyChecking = false;
					currentState[7][5].piece = WHITE_ROOK;
					currentState[7][7].piece = NONE;
					whiteKing.currentIndex = indexWish;
					return 0;
				}
			}
			if (whiteKing.canCastleLong && (wishL == 3)) {
				for (char j = 2; j < 5; j++) {
					if (currentState[7][j - 1].piece == NONE) {}
					else { return 1; }
				}
				if (checkForChecks(whiteKing.currentIndex) || checkForChecks(indexWish) || checkForChecks(0x84)) {
					bAlreadyChecking = false;
					return 1;
				}
				else {
					bAlreadyChecking = false;
					currentState[7][3].piece = WHITE_ROOK;
					currentState[7][0].piece = NONE;
					whiteKing.currentIndex = indexWish;
					return 0;
				}
			}
		}
		else {
			return 1;
		}
	}
	if (bCurrentTurn) {
		if (legalAttempt) {
			
			blackKing.canCastle = false;
			blackKing.wantsToMove = true;
			return 0;
		}
		else if (blackKing.canCastle) {
			if (blackKing.canCastleShort && (wishL == 7)) {
				for (char j = 7; j > 5; j--) {
					if (currentState[0][j - 1].piece == NONE) {}
					else { return 1; }
				}
				if (checkForChecks(blackKing.currentIndex) || checkForChecks(indexWish) || checkForChecks(0x16)) {
					bAlreadyChecking = false;
					return 1;
				}
				else {
					bAlreadyChecking = false;
					currentState[0][5].piece = BLACK_ROOK;
					currentState[0][7].piece = NONE;
					blackKing.currentIndex = indexWish;
					return 0;
				}
			}
			if (blackKing.canCastleLong && (wishL == 3)) {
				for (char j = 2; j < 5; j++) {
					if (currentState[0][j - 1].piece == NONE) {}
					else { return 1; }
				}
				if (checkForChecks(blackKing.currentIndex) || checkForChecks(indexWish) || checkForChecks(0x14)) {
					bAlreadyChecking = false;
					return 1;
				}
				else {
					bAlreadyChecking = false;
					currentState[0][3].piece = BLACK_ROOK;
					currentState[0][0].piece = NONE;
					blackKing.currentIndex = indexWish;
					return 0;
				}
			}
		}
		else {
			return 1;
		}
	}

	/* never gets here */
	return 1;
}

bool canMoveThere(char indexWish) {

	switch (movingPiece.piece) {
	default:
		return true;
	case(WHITE_PAWN):
		if (1 == validatePawnMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(BLACK_PAWN):
		if (1 == validatePawnMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(WHITE_KNIGHT):
		if (1 == validateKnightMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(BLACK_KNIGHT):
		if (1 == validateKnightMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(WHITE_BISHOP):
		if (1 == validateBishopMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(BLACK_BISHOP):
		if (1 == validateBishopMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(WHITE_ROOK):
		if (1 == validateRookMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(BLACK_ROOK):
		if (1 == validateRookMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(WHITE_QUEEN):
		if (1 == validateQueenMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(BLACK_QUEEN):
		if (1 == validateQueenMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(WHITE_KING):
		if (1 == validateKingMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	case(BLACK_KING):
		if (1 == validateKingMove(indexWish, movingPiece.indexOrg)) { break; }
		return false;
	}
	return true;
}

int checkLegalMove(char indexWish, chessPiece_e* piece) {

	char v = ((indexWish & 0xf0) >> 4) - 1;
	char u = (indexWish & 0x0f) - 1;
	

	if (bCurrentTurn && isBlack(currentState[v][u].piece, piece)) { /* black turn and wish piece is anything but black */
		if(!canMoveThere(indexWish)){
			return 0;
		}
	}
	if (!bCurrentTurn && isWhite(currentState[v][u].piece, piece)) { /* white turn and wish piece is anything but white */
		if (!canMoveThere(indexWish)) {
			return 0;
		}
	}

	return 1;
}

void handleEnPassant(char dest) {

	char v;
	if (!bCurrentTurn) {
		v = ((dest & 0xf0) >> 4);
	}
	else {
		v = ((dest & 0xf0) >> 4) - 2;
	}
	char u = (dest & 0x0f) - 1;

	
	currentState[v][u].piece = NONE;
}

void getTemporaryState(void) {
	
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			temporaryState[i][j].piece = currentState[i][j].piece;
		}
	}
}

void revertState(void) {

	if (blackKing.wantsToMove) { /* Needs To Be Reverted If It Was Changed */

		blackKing.wantsToMove = false;
		blackKing.currentIndex = movingPiece.indexOrg;
	}
	if (whiteKing.wantsToMove) {

		whiteKing.wantsToMove = false;
		whiteKing.currentIndex = movingPiece.indexOrg;
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			currentState[i][j].piece = temporaryState[i][j].piece;
		}
	}
}

void movePiece(char dest) {

	char v = ((dest & 0xf0) >> 4) - 1;
	char u = (dest & 0x0f) - 1;

	currentState[v][u].piece = movingPiece.piece;
}

void endPlayerMove(void) {
	
	if (!movingPiece.exists) { return; } /* Make Sure That A Valid Move Was Started */
	movingPiece.exists = false;
	bHandleEnPassant = false; /* FixMe: Seems Ugly To Do This Every Move */
	char indexWish = 0x00;
	double posx = mousePosition.x;
	double posy = mousePosition.y;
	chessPiece_e pieceAtDest = NONE;

	getTemporaryState(); /* For Reverting The State */
	indexWish = getChessIndex(posx, posy);
	if (indexWish == movingPiece.indexOrg)            { illegalMove(); return; } /* Already There */
	if (indexWish == 0x00)                            { illegalMove(); return; } /* Out Of Bounds */
	if (checkLegalMove(indexWish, &pieceAtDest) == 1) { illegalMove(); return; }

	movePiece(indexWish);

	if (blackKing.wantsToMove) { /* Only Used If Moving Piece Is King, Needed To Make Sure Cannot self-check */

		blackKing.wantsToMove = false;
		blackKing.currentIndex = indexWish;
	}
	if (whiteKing.wantsToMove) {

		whiteKing.wantsToMove = false;
		whiteKing.currentIndex = indexWish;
	}

	if (!bCurrentTurn) { /* Too Late For Illegal Move Function To Revert At This Point, So Use revertState() Instead */
		if (checkForChecks(whiteKing.currentIndex)) { bAlreadyChecking = false; bWantsToPromote = false; revertState(); return; }
	}
	else {
		if (checkForChecks(blackKing.currentIndex)) { bAlreadyChecking = false; bWantsToPromote = false; revertState(); return; }
	}
	bAlreadyChecking = false;

	if (bWantsToPromote) {
		bWantsToPromote = false;
		bValidPromotion = true;
		movingPiece.indexOrg = indexWish;
	}

	iCanEnPassant--; /* FixMe: Seems Ugly To Do This Every Move */
	
	if (bHandleEnPassant == true) {
		handleEnPassant(indexWish);
	}
	movingPiece.piece = NONE;
	bCurrentTurn = !bCurrentTurn;
}

void promoteToQueen(void) {

	char v = ((movingPiece.indexOrg & 0xf0) >> 4) - 1; /* We Changed Org To Wish When We Set The Promotion Flag */
	char u = (movingPiece.indexOrg & 0x0f) - 1;

	if (bCurrentTurn) { /* We're Promoting On Opponents Turn */
		currentState[v][u].piece = WHITE_QUEEN;
	}
	else {
		currentState[v][u].piece = BLACK_QUEEN;
	}
	bValidPromotion = false; /* We Can Continue Normally */
}

void promoteToRook(void) {

	char v = ((movingPiece.indexOrg & 0xf0) >> 4) - 1;
	char u = (movingPiece.indexOrg & 0x0f) - 1;

	if (bCurrentTurn) {
		currentState[v][u].piece = WHITE_ROOK;
	}
	else {
		currentState[v][u].piece = BLACK_ROOK;
	}
	bValidPromotion = false;
}

void promoteToKnight(void) {

	char v = ((movingPiece.indexOrg & 0xf0) >> 4) - 1;
	char u = (movingPiece.indexOrg & 0x0f) - 1;

	if (bCurrentTurn) { 
		currentState[v][u].piece = WHITE_KNIGHT;
	}
	else {
		currentState[v][u].piece = BLACK_KNIGHT;
	}
	bValidPromotion = false; 
}

void promoteToBishop(void) {

	char v = ((movingPiece.indexOrg & 0xf0) >> 4) - 1;
	char u = (movingPiece.indexOrg & 0x0f) - 1;

	if (bCurrentTurn) { 
		currentState[v][u].piece = WHITE_BISHOP;
	}
	else {
		currentState[v][u].piece = BLACK_BISHOP;
	}
	bValidPromotion = false;
}


 /* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
  *                                                                                                                            *
  *            GLFW Callbacks                                                                                                  *
  *	                                                                                                                          *
  * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		bScreenIsFlipped = !bScreenIsFlipped;
	}

	if (bValidPromotion) {
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
			promoteToQueen();
		if (key == GLFW_KEY_R && action == GLFW_PRESS)
			promoteToRook();
		if (key == GLFW_KEY_N && action == GLFW_PRESS)
			promoteToKnight();
		if (key == GLFW_KEY_B && action == GLFW_PRESS)
			promoteToBishop();
	}
}

void mouse_button_disabled(GLFWwindow* window, int button, int action, int mods) { /* does what it says */
	
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		startPlayerMove();
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		endPlayerMove();
}

void windowResize(GLFWwindow* window, int width, int height) {

	if (width > height) {

		long shift = (long)(width - height);


		mousePosition.offsetY = 0.0f;
		mousePosition.offsetX = (double)(shift >> 1);

		glViewport((GLint)(shift >> 1), 0, height, height);

		windowResolution.x = (long)height;
		windowResolution.y = (long)height;
	}
	else if (width < height) {

		long shift = (long)(height - width);


		mousePosition.offsetX = 0.0f;
		mousePosition.offsetY = (double)(shift >> 1);

		glViewport(0, (GLint)(shift >> 1), width, width);

		windowResolution.x = (long)width;
		windowResolution.y = (long)width;
	}
	else {

		mousePosition.offsetX = 0.0f;
		mousePosition.offsetY = 0.0f;
		
		glViewport(0, 0, width, height);

		windowResolution.x = (long)width;
		windowResolution.y = (long)height;
	}
}


/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
 *                                                                                                                            *
 *            Timing                                                                                                          *
 *	                                                                                                                          *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

int Sys_Milliseconds(windowsTimer_s* t) {

	int sys_curtime;
	static bool initialized = false;


	if (!initialized) {

		t->time = timeGetTime(); // winmm.lib
		initialized = true;
	}
	sys_curtime = timeGetTime() - t->time;


	return sys_curtime;
}

void timerStart(windowsTimer_s* t) {

	t->start = Sys_Milliseconds(t);
}

void timerEnd(windowsTimer_s* t) {

	t->end = Sys_Milliseconds(t);
	t->ms += t->end - t->start;
}


/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ *
 *                                                                                                                            *
 *            Main                                                                                                            *
 *	                                                                                                                          *
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

int main(int argc, char* argv[]) {

	windowsTimer_s winTimer = {};


	timerStart(&winTimer);


    /* oooooooooooooooooooooooooooooooooooooooooooooooooooo GLFW -- Init oooooooooooooooooooooooooooooooooooooooooooooooooooo */

	GLFWwindow* window;


    if (!glfwInit()) return 1;
        
	window = glfwCreateWindow(windowResolution.x, windowResolution.y, "jonochess", NULL, NULL);
    if (!window) {glfwTerminate();return 1;}

    glfwMakeContextCurrent(window);

	/* Callbacks */
	GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursor(window, cursor);
	
	glfwSetFramebufferSizeCallback(window, windowResize);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetKeyCallback(window, key_callback);


    /* oooooooooooooooooooooooooooooooooooooooooooooooooooo GLEW -- Init oooooooooooooooooooooooooooooooooooooooooooooooooooo */
    
	GLenum err = glewInit();


    if (GLEW_OK != err) { fprintf(stderr, "Error: %s\n", glewGetErrorString(err)); glfwTerminate(); return 1;}

    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));    
    

	/* ooooooooooooooooooooooooooooooooooooooooooooooooooo Shader -- Init ooooooooooooooooooooooooooooooooooooooooooooooooooo */

	shaderBuffer_s chessBoardShader = {};
	shaderBuffer_s chessPieceDynamicShader = {};
	shaderBuffer_s chessPieceStaticShader = {};


	InitShader("res/shader/board.vs", "res/shader/board.fs", &chessBoardShader);
	InitShader("res/shader/piece.vs", "res/shader/piece.fs", &chessPieceDynamicShader);
	InitShader("res/shader/grid.vs", "res/shader/grid.fs", &chessPieceStaticShader);

	/* Used To Bind A Chess Piece To Mouse Cursor */
	uniformMat4_s transform = {}; transform.name = "transform";
	transform.location = glGetUniformLocation(chessPieceDynamicShader.program, transform.name);

	GLint TexCoordShiftLoc1 = glGetUniformLocation(chessPieceDynamicShader.program, "TexCoordShift1");

	/* Used To Index The Board Coordinates And Draw Static Pieces To Correct Squares */
	uniformMat4_s transform1 = {}; transform1.name = "transform1";
	transform1.location = glGetUniformLocation(chessPieceStaticShader.program, transform1.name);

	GLint TexCoordShiftLoc = glGetUniformLocation(chessPieceStaticShader.program, "TexCoordShift");

	
	/* oooooooooooooooooooooooooooooooooooooooooooooooooo Texture -- Init ooooooooooooooooooooooooooooooooooooooooooooooooooo */

	textureBuffer_s texture0 = {}; texture0.name = "texture0";
	textureBuffer_s texture1 = {}; texture1.name = "texture0";

	InitTexture("res/texture/chesspiece.png", &texture0);
	InitTexture("res/texture/chesspieceflip.png", &texture1);

	/* Set Texture Uniforms */
	shaderUse(chessPieceStaticShader.program); /* don't forget to activate/use the shader before setting uniforms! */
	setTextureUniform(&chessPieceStaticShader, &texture0);

	shaderUse(chessPieceDynamicShader.program);
	setTextureUniform(&chessPieceDynamicShader, &texture0);


	/* ooooooooooooooooooooooooooooooooooooooooooooooooooo Object -- Init ooooooooooooooooooooooooooooooooooooooooooooooooooo */
	
	objectBuffer_s chessBoardObject = {};
	objectBuffer_s chessPieceDynamicObject = {};
	objectBuffer_s chessPieceStaticObject = {};


	InitObject("res/obj/chessboard.obj", &chessBoardObject);
	InitObject("res/obj/chesspiece.obj", &chessPieceDynamicObject);
	InitObject("res/obj/chessgrid.obj", &chessPieceStaticObject);


	/* oooooooooooooooooooooooooooooooooooooooooooooooooo Game Logic -- Init oooooooooooooooooooooooooooooooooooooooooooooooo */
	
	blackKing.currentIndex = 0x15; whiteKing.currentIndex = 0x85;
	
	/* TODO: Make A Board From Black Or White Perspective */


	/* oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo */

	/* Timing */
	timerEnd(&winTimer);
    fprintf(stdout, "Status: Init complete in %u ms\n", winTimer.ms);


	/* oooooooooooooooooooooooooooooooooooooooooooooooooooooo Main Loop ooooooooooooooooooooooooooooooooooooooooooooooooooooo */
    while (!glfwWindowShouldClose(window)) {
		
		/* Promoting */
		while (bValidPromotion) {
			glfwSetMouseButtonCallback(window, mouse_button_disabled);
			glfwWaitEvents();
			glfwSetMouseButtonCallback(window, mouse_button_callback);
		}

        /* Timing */
		timerStart(&winTimer);

		/* Mouse */
		glfwGetCursorPos(window, &mousePosition.x, &mousePosition.y);
		mousePosition.x -= mousePosition.offsetX; // TLDR; glfw gives us screen coords but we need viewport coords, see callbacks for more info
		mousePosition.y -= mousePosition.offsetY;
		mousePosition.x /= (windowResolution.x >> 1); mousePosition.y /= (windowResolution.y >> 1); mousePosition.x -= 1.0f; mousePosition.y -= 1.0f;

        /* Refresh Frame */
		glClearColor(0.7f, 0.5f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		/* Bind Textures*/
		shaderUse(chessPieceStaticShader.program); /* don't forget to activate/use the shader before setting uniforms! */
		if (bScreenIsFlipped) {
			setTextureUniform(&chessPieceStaticShader, &texture1);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture1.id);
		}
		else {
			setTextureUniform(&chessPieceStaticShader, &texture0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture0.id);
		}
		
		/* Board */
		shaderUse(chessBoardShader.program);
		drawTris(&chessBoardObject);
		
		/* Static Pieces */
		shaderUse(chessPieceStaticShader.program);
		drawChessPieceStatic(&transform1, TexCoordShiftLoc, &chessPieceStaticObject);
		
		/* Dynamic Piece */
		if (movingPiece.exists) {
			shaderUse(chessPieceDynamicShader.program);
			drawChessPieceDynamic(&transform, TexCoordShiftLoc1, &chessPieceDynamicObject);
		}

        /* End Frame */
        glfwSwapBuffers(window);
        glfwPollEvents();

        /* Timing */
		timerEnd(&winTimer);

		if (!movingPiece.exists) {
			glfwWaitEvents();
		}
    }

	/* Cleanup */
	freeBufferObject(&chessBoardObject);
	freeBufferObject(&chessPieceStaticObject);
	freeBufferObject(&chessPieceDynamicObject);
	
    glfwTerminate();


    return 0;
}