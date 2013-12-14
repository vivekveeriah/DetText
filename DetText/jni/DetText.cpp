#include <com_vivek_dettext_MainActivity.h>

double delta = 12.5;
double minDiversity = 0.9;
double maxVariation = 0.3;
float maxStrokeWidth = 100;
float initialStrokeWidth = 0;

JNIEXPORT void JNICALL Java_com_vivek_dettext_MainActivity_detectText(JNIEnv * env, jobject obj, jlong inputFrame, jlong ProcFrame)
{
	Mat segMat, segMatConv;
	Mat &inpMat = * (Mat * )inputFrame;
	equalizeHist(inpMat, inpMat);
	Mat decMat = adaptiveDecomposition(inpMat);
	Mat mserMat = extractMSER(inpMat, delta, minDiversity, maxVariation);
	bitwise_and(mserMat, decMat, segMat);
	segMat.convertTo(segMatConv, CV_8UC1);
	Mat swtMap = strokeWidthTransform(segMatConv, 1); // Set Proper Search Direction

	inpMat = swtMap;
}

Mat adaptiveDecomposition(Mat srcMat)
{
	Mat filteredMat, posThresh, negThresh;
	Mat decMat = Mat(srcMat.size(), CV_8UC1);
	boxFilter(srcMat, filteredMat, -1, Size(3,3));
	add(filteredMat, 10, posThresh);
	add(filteredMat, -10, negThresh);
	Mat pos = posThresh - srcMat;
	Mat neg = srcMat - negThresh;
	threshold(pos, pos, 0, 255, CV_THRESH_BINARY);
	threshold(neg, neg, 0, 100, CV_THRESH_BINARY_INV);
	bitwise_or(pos, neg, decMat);

	return decMat;
}

Mat extractMSER(Mat srcMat, double delta, double minDiversity, double maxVariation)
{
	vector<vector<Point> > regions;
	MSER(delta, 60, 14400, maxVariation, minDiversity)(srcMat, regions);
	Mat mask = Mat::zeros(srcMat.size(), CV_8UC1);
	for(vector<vector<Point> > :: iterator rit = regions.begin(); rit != regions.end(); rit++)
	{
		for(vector<Point> :: iterator rit1 = rit->begin(); rit1 != rit->end(); rit1++)
		{
			Point intpoint = Point(*rit1);
			mask.at<uchar>(intpoint.y, intpoint.x) = (uchar)255;
		}
	}
	return mask;
}


Mat strokeWidthTransform(Mat srcMat, int searchDirection)
{
	//-----To Do : Initialize all MAT before use
	vector<Point> edgePoints;
	Mat edgeMap;
	Mat swtMap = Mat::zeros(srcMat.size(), CV_32FC1);
	Canny(srcMat, edgeMap, 50, 120);
	Mat dx, dy;
	Sobel(srcMat, dx, CV_32FC1, 1, 0, 3);
	Sobel(srcMat, dy, CV_32FC1, 0, 1, 3);
	Mat theta = Mat(srcMat.size(), CV_32FC1);
	for(int y = 0; y < srcMat.rows; y++)
		for(int x = 0; x < srcMat.cols; x++)
		{
			if(edgeMap.at<uchar>(y,x) == 255)
			{
				theta.at<float>(y,x) = atan2(dy.at<float>(y,x), dx.at<float>(y,x));
				edgePoints.push_back(Point(x,y));
			}
		}
	vector<Point> strokePoints;
	updateStrokeWidthTransform(swtMap, theta, edgeMap, edgePoints, strokePoints, searchDirection, 0); //UPDATE
	updateStrokeWidthTransform(swtMap, theta, edgeMap, edgePoints, strokePoints, searchDirection, 1); //REFINE
	return swtMap;
}

void updateStrokeWidthTransform(Mat & swtMap,Mat & theta, Mat & edgeMap, vector<Point> & edgePoints, vector<Point> & strokePoints, int searchDirection, int purpose)
{
	vector<Point> :: iterator itr = edgePoints.begin();
	vector<Point> pointStack;
	vector<float> swtValues;
	for(; itr != edgePoints.end(); itr++)
	{
		pointStack.clear();
		swtValues.clear();
		float step = 1;
		float iy = (*itr).y;
		float ix = (*itr).x;
		float currY = iy;
		float currX = ix;
		bool isStroke = false;
		float iTheta = theta.at<float>(*itr);
		while(step < maxStrokeWidth)
		{
			float nextY = round(iy + sin(iTheta) * step * searchDirection);
			float nextX = round(ix + sin(iTheta) * step * searchDirection);
			if(nextY < 0 || nextX < 0 || nextY >= swtMap.rows || nextX >= swtMap.cols)
				break;
			step = step + 1;
			if(currX == nextX || currY == nextY)
				continue;
			currX = nextX;
			currY = nextY;
			pointStack.push_back(Point(currY, currX));
			swtValues.push_back(swtMap.at<float>(currY, currX));
			if(edgeMap.at<uchar>(currY, currX) == 255)
			{
				float jTheta = theta.at<float>(currY, currX);
				if(abs(abs(iTheta - jTheta - 3.14)) < 3.14 / 2)
				{
					isStroke = true;
					if(purpose == 0) // UPDATE
					{
						strokePoints.push_back(Point(iy, ix));
					}
				}
				break;
			}
			if(isStroke)
			{
				float newSwtValue;
				if(purpose == 0) //UPDATE
				{
					newSwtValue = sqrt(((currY - iy) * (currY - iy)) + ((currX - ix) * (currX - ix)));
				}
				else if (purpose == 1) //REFINE
				{
					std::nth_element(swtValues.begin(), swtValues.begin() + swtValues.size() / 2, swtValues.end());
					newSwtValue = swtValues[swtValues.size() / 2];
				}
				for(int i = 0; i < pointStack.size(); i++)
				{
					swtMap.at<float>(pointStack[i]) = std::min(swtMap.at<float>(pointStack[i]), newSwtValue);
				}
			}
		}
	}//End Loop of Edge Points
	for(int y = 0; y < swtMap.rows; y++)
		for(int x = 0; x < swtMap.cols; x++)
		{
			if(swtMap.at<float>(y,x) == initialStrokeWidth)
			{
				swtMap.at<float>(y,x) = 0;
			}
		}
}

Mat connectedComponentAnalysis(Mat & swtMap)
{
	Mat ccMap = Mat::zeros(swtMap.size(), CV_32FC1);
}
