// minigame.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>				// windows��ͷ�ļ�
#include <gl/glut.h>				// Glut��ͷ�ļ�
#include "cv.h"
#include "highgui.h"
#include "gesrec.h"
#include <stdio.h>

// ���崰�ڵı��⡢���ȡ��߶�
#define WIN_TITLE "Minigame"
const int WIN_WIDTH=400;
const int WIN_HEIGHT=400;
BOOL isFullScreen = FALSE;

// �û���������
CvCapture* capture = 0;
CvMemStorage* compSto;
CvSeq* compSeq;
CvMemStorage* templateContoursSto;
CvSeq* templateContoursSeq;
CvSeq* rectSeq;//�洢��������
CvMemStorage* rectSto;
IplImage* camInput;
IplImage* camOutput = 0;
IplImage* camContour;
IplImage* templateContourImg = 0;
CvRect head;//ͷ
CvRect rightHand;//����
CvRect leftHand;//����
float	yrot;
int rect_num = 0;

// OpenGL��ʼ��
void InitGL(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_CULL_FACE);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void loadTemplate(char* filename, IplImage* templateContourImg, CvSeq* templateContours)
{
	IplImage* templateInput;
	CvMemStorage* a_templateContourSto;
	CvSeq* a_templateContourSeq;

	a_templateContourSto = cvCreateMemStorage(0);
	a_templateContourSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), a_templateContourSto);

	templateInput = cvLoadImage(filename, 1);
	templateContourImg = cvCreateImage(cvGetSize(templateInput), IPL_DEPTH_8U, 3);

	gesFindContours(templateInput, templateContourImg, &a_templateContourSeq, a_templateContourSto, 1);

	//��һ��ģ�����
	cvSeqPush(templateContours, a_templateContourSeq);
	printf("ContourNum:%d\n", templateContours->total);

	//�ͷ�ģ���ڴ�
	cvReleaseImage(&templateInput);
	cvReleaseImage(&templateContourImg);
	//cvReleaseMemStorage(&a_templateContourSto);
}

// OpenCV��ʼ��
void InitCV(void)
{
	capture = cvCaptureFromCAM(0);
	if(!capture)
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return;
	}

	//����ģ��
	compSto = cvCreateMemStorage(0);
	compSeq = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), compSto);
	templateContoursSto = cvCreateMemStorage(0);
	templateContoursSeq = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvSeq), templateContoursSto);
	
	loadTemplate("myskin1.jpg", templateContourImg, templateContoursSeq);
	loadTemplate("myskin2.jpg", templateContourImg, templateContoursSeq);
	loadTemplate("myskin3.jpg", templateContourImg, templateContoursSeq);
	//loadTemplate("myskin4.jpg", templateContourImg, templateContoursSeq);/////////////////////��1�����
	//loadTemplate("myskin5.jpg", templateContourImg, templateContoursSeq);/////////////////////��3�����

	//��õ�һ֡
	camInput = cvQueryFrame(capture);
	if(!camInput)
	{
		return;
	}
	camContour = cvCreateImage(cvGetSize(camInput), IPL_DEPTH_8U, 3);

	//��ʼ�������þ��δ洢
	rectSto = cvCreateMemStorage(0);
	rectSeq = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), rectSto);

	cvNamedWindow("CamInput", 1);
	cvNamedWindow("CamOutput", 1);
	cvNamedWindow("CamContour", 1);
}

// OpenCV�ڴ��ͷ�
void ReleaseCV(void)
{
	cvReleaseCapture(&capture);
	cvReleaseImage(&camOutput);
	cvReleaseImage(&camContour);
	cvDestroyWindow("CamInout");
	cvDestroyWindow("CamOutput");
	cvDestroyWindow("CamContour");
	cvReleaseMemStorage(&templateContoursSto);
	cvReleaseMemStorage(&compSto);
	cvReleaseMemStorage(&rectSto);
}

//��ʼ�����������ͷ
void Detect()
{
	CvConnectedComp* comp;
	CvRect temp;

	//�õ���ǰ֡�Ķ�ֵ�����
	gesDetectHandRange(camInput, camOutput, compSeq);

	rect_num = compSeq->total;
		
	if(rect_num < 3)
	{
		printf("Can't find all three");
		return;
	}

	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, 0);
	head = comp->rect;
	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, 1);
	rightHand = comp->rect;
	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, 2);
	leftHand = comp->rect;
	
	//���ݾ�������ȷ����ʼʱͷ��������λ��
	if((head.y+head.height/2) > (rightHand.y+rightHand.height/2))
	{
		temp = head;
		head = rightHand;
		rightHand = temp;
	}
	if((head.y+head.height/2) > (leftHand.y+leftHand.height/2))
	{
		temp = head;
		head = leftHand;
		leftHand = temp;
	}
	if((rightHand.x+rightHand.width/2) > (leftHand.x+leftHand.width/2))
	{
		temp = rightHand;
		rightHand = leftHand;
		leftHand = temp;
	}

	//�ò�ͬ��ɫ�ľ��ηֱ��ʾ������
	cvRectangle(camInput, cvPoint(head.x, head.y),
					cvPoint(head.x + head.width, head.y + head.height), 
					cvScalar(255, 255, 255), 1);
	cvRectangle(camInput, cvPoint(rightHand.x, rightHand.y),
					cvPoint(rightHand.x + rightHand.width, rightHand.y + rightHand.height), 
					cvScalar(0, 255, 0), 1);
	cvRectangle(camInput, cvPoint(leftHand.x, leftHand.y),
					cvPoint(leftHand.x + leftHand.width, leftHand.y + leftHand.height), 
					cvScalar(0, 0, 255), 1);

	cvShowImage("CamInput", camInput);
}

void Track()
{
	CvConnectedComp* comp;

	//���и���
	gesTracking(camInput, camOutput, compSeq, rectSeq);

	cvClearSeq(compSeq);
	compSeq = cvCloneSeq(rectSeq);

	rect_num = compSeq->total;

	if(rect_num < 3)
	{
		printf("Can't find all three");
		return;
	}

	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, 0);
	head = comp->rect;
	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, 1);
	rightHand = comp->rect;
	comp = (CvConnectedComp*)cvGetSeqElem(compSeq, 2);
	leftHand = comp->rect;

	//�ò�ͬ��ɫ�ľ��ηֱ��ʾ������
	cvRectangle(camInput, cvPoint(head.x, head.y),
					cvPoint(head.x + head.width, head.y + head.height), 
					cvScalar(255, 255, 255), 1);
	cvRectangle(camInput, cvPoint(rightHand.x, rightHand.y),
					cvPoint(rightHand.x + rightHand.width, rightHand.y + rightHand.height), 
					cvScalar(0, 255, 0), 1);
	cvRectangle(camInput, cvPoint(leftHand.x, leftHand.y),
					cvPoint(leftHand.x + leftHand.width, leftHand.y + leftHand.height), 
					cvScalar(0, 0, 255), 1);

	cvShowImage("CamInput", camInput);
}

// �������ƺ���
void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// ����һ����ת������׶��
	glTranslatef(0.0f,0.0f,-10.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);

	glBegin(GL_TRIANGLE_FAN);
		glColor3f(1.0f,0.0f,0.0f); glVertex3f( 0.0f, 1.0f, 0.0f);

		glColor3f(0.0f,1.0f,0.0f); glVertex3f(-1.0f,-1.0f, 1.0f);
		glColor3f(0.0f,0.0f,1.0f); glVertex3f( 1.0f,-1.0f, 1.0f);
		glColor3f(0.0f,1.0f,0.0f); glVertex3f( 1.0f,-1.0f,-1.0f);
		glColor3f(0.0f,0.0f,1.0f); glVertex3f(-1.0f,-1.0f,-1.0f);
		glColor3f(0.0f,1.0f,0.0f); glVertex3f(-1.0f,-1.0f, 1.0f);
	glEnd();

	yrot+=0.15f;

	glutSwapBuffers ( );	// ����˫����
}

// ���ô��ڸı��Сʱ�Ĵ�������
void Reshape(int width, int height)
{
	if (height==0)
	{
		height=1;
	}

	// �����ӿ�
	glViewport(0,0,width,height);

	// ����ͶӰ����
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// ���̰��������Ĵ����������ж�ӦASCII��İ�����
void Keyboard (unsigned char key, int x, int y)
{
	switch(key) {
		case 27:				// ��ESC��ʱ�˳�����
			ReleaseCV();
			exit (0);
			break;
		default:
			break;
	}
}

// ���̰��������Ĵ����������޶�ӦASCII��İ�����
void arrow_keys(int a_keys, int x, int y)
{
	switch(a_keys) {
		case GLUT_KEY_F1:		// ��F1��ʱ�л�����/ȫ��ģʽ
			if(isFullScreen)
			{
				glutReshapeWindow(WIN_WIDTH, WIN_HEIGHT);
				glutPositionWindow(30, 30);
				isFullScreen = FALSE;
			}
			else
			{
				glutFullScreen();
				isFullScreen = TRUE;
			}
			break;
		default:
			break;
	}
}

void Timer(int)
{
	/*camInput = cvQueryFrame(capture);
	if(!camInput)
	{
		return;
	}
	
	cvReleaseImage(&camOutput);
	camOutput = cvCloneImage(camInput);

	gesDetectHandRange(camInput, camOutput, compSeq);
	
	Detect();

	int result = gesMatchContoursTemplate2(camOutput, camContour, templateContoursSeq);

	if(result == 0)
	{
		yrot += 1;
	}
	else if(result == 1)
	{
		yrot -= 1;
	}

	cvShowImage("CamOutput", camOutput);
	cvShowImage("CamContour", camContour);

	glutPostRedisplay();
	glutTimerFunc(33, Timer, 0);*/
	
	//�õ���ǰ֡
	camInput = cvQueryFrame(capture);

	//�ͷ�ǰһ֡�Ķ�ֵ�����
	cvReleaseImage(&camOutput);
	camOutput = cvCloneImage(camInput);

	printf("%d\n", rect_num);

	if(rect_num < 3)
	{
		Detect();
	}
	else
	{
		Track();
	}

	//cvShowImage("CamInput", camInput);

	glutTimerFunc(33, Timer, 0);
}

// ����������
void main(int argc, char** argv)
{
	glutInit(&argc, argv);										// ��ʼ��GLUT��

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);	// ��ʼ����ʾģʽ

	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);					// ��ʼ�����ڴ�С
	glutInitWindowPosition(30,30);								// ��ʼ������λ��
	glutCreateWindow(WIN_TITLE);								// ��������

	InitGL();					// ��ʼ��OpenGL

	InitCV();					// ��ʼ��OpenCV

	glutDisplayFunc(Display);	// ���ô�����ʾʱ���õĴ�������

	glutReshapeFunc(Reshape);	// ���ô��ڸı��Сʱ�Ĵ�������

	glutKeyboardFunc(Keyboard);	// ���ü��̰��������Ĵ����������ж�ӦASCII��İ�����
	glutSpecialFunc(arrow_keys);// ���ü��̰��������Ĵ����������޶�ӦASCII��İ�����

	glutTimerFunc(33, Timer, 0);

	glutMainLoop();				// ����GLUT�¼�����ѭ��
}