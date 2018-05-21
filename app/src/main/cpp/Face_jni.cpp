#include <stdio.h>
#include <jni.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <opencv2/opencv.hpp>
#include <android/log.h>
/* Header for class com_simoncherry_jnidemo_JNIUtils */
#include "Face_jni.h"
#include "json/json.h"
#include "md5.h"

#define LOG    "Face-jni" // 这个是自定义的LOG的标识
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...)  __android_log_print(ANDROID_LOG_FATAL,LOG,__VA_ARGS__) // 定义LOGF类型

#ifdef __cplusplus
extern "C" {
#endif

using namespace cv;
using namespace std;
IplImage * change4channelTo3InIplImage(IplImage * src);

IplImage * change4channelTo3InIplImage(IplImage * src) {
    if (src->nChannels != 4) {
        return NULL;
    }

    IplImage * destImg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
    for (int row = 0; row < src->height; row++) {
        for (int col = 0; col < src->width; col++) {
            CvScalar s = cvGet2D(src, row, col);
            cvSet2D(destImg, row, col, s);
        }
    }

    return destImg;
}

IplImage * changeFuckIplImage(IplImage * src) {
    IplImage * destImg = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
    for (int row = 0; row < src->height; row++) {
        for (int col = 0; col < src->width; col++) {
            CvScalar s = cvGet2D(src, row, col);
            cvSet2D(destImg, row, col, s);
        }
    }

    return destImg;
}

/*
 * Class:     com_simoncherry_averageface_JNIUtils
 * Method:    doGrayScale 图像灰度化
 * Signature: ([III)[I
 */
JNIEXPORT jintArray JNICALL Java_com_lichao_opencv4androidface_uitls_JNIUtils_doGrayScale
        (JNIEnv *env, jclass obj, jintArray buf, jint w, jint h) {
    LOGE("doGrayScale Start");

    jint *cbuf;
    cbuf = env->GetIntArrayElements(buf, JNI_FALSE);
    if (cbuf == NULL) {
        return 0;
    }

    cv::Mat imgData(h, w, CV_8UC4, (unsigned char *) cbuf);

    uchar* ptr = imgData.ptr(0);
    for(int i = 0; i < w*h; i ++){
        //计算公式：Y(亮度) = 0.299*R + 0.587*G + 0.114*B
        //对于一个int四字节，其彩色值存储方式为：BGRA
        int grayScale = (int)(ptr[4*i+2]*0.299 + ptr[4*i+1]*0.587 + ptr[4*i+0]*0.114);
        ptr[4*i+1] = grayScale;
        ptr[4*i+2] = grayScale;
        ptr[4*i+0] = grayScale;
    }

    int size = w * h;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, cbuf);
    env->ReleaseIntArrayElements(buf, cbuf, 0);

    LOGE("doGrayScale End");
    return result;
}

/*
 * Class:     com_simoncherry_averageface_JNIUtils
 * Method:    doEdgeDetection 图像边缘检测
 * Signature: ([III)[I
 */
JNIEXPORT jintArray JNICALL Java_com_lichao_opencv4androidface_uitls_JNIUtils_doEdgeDetection
        (JNIEnv *env, jclass obj, jintArray buf, jint w, jint h) {
    jint *cbuf;
    cbuf = env->GetIntArrayElements(buf, JNI_FALSE);
    if (cbuf == NULL) {
        return 0;
    }

    cv::Mat myimg(h, w, CV_8UC4, (unsigned char*) cbuf);
    IplImage image = IplImage(myimg);
    IplImage* image3channel = change4channelTo3InIplImage(&image);

    IplImage* pCannyImage = cvCreateImage(cvGetSize(image3channel),IPL_DEPTH_8U,1);
    cvCanny(image3channel, pCannyImage, 50, 150, 3);

    int* outImage = new int[w*h];
    for(int i=0;i<w*h;i++) {
        outImage[i]=(int)pCannyImage->imageData[i];
    }

    int size = w * h;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, outImage);
    env->ReleaseIntArrayElements(buf, cbuf, 0);
    return result;
}

/*
 * Class:     com_simoncherry_averageface_JNIUtils
 * Method:    doBinaryzation 图像二值化
 * Signature: ([III)[I
 */
JNIEXPORT jintArray JNICALL Java_com_lichao_opencv4androidface_uitls_JNIUtils_doBinaryzation
        (JNIEnv *env, jclass obj, jintArray buf, jint w, jint h) {
    jint *cbuf;
    cbuf = env->GetIntArrayElements(buf, JNI_FALSE);
    if (cbuf == NULL) {
        return 0;
    }

    cv::Mat imgData(h, w, CV_8UC4, (unsigned char *) cbuf);
    IplImage image = IplImage(imgData);
    IplImage *pSrcImage = change4channelTo3InIplImage(&image);
    IplImage *g_pGrayImage = cvCreateImage(cvGetSize(pSrcImage), IPL_DEPTH_8U, 1);
    cvCvtColor(pSrcImage, g_pGrayImage, CV_BGR2GRAY);

    IplImage *g_pBinaryImage = cvCreateImage(cvGetSize(g_pGrayImage), IPL_DEPTH_8U, 1);
    cvThreshold(g_pGrayImage, g_pBinaryImage, 127, 255, CV_THRESH_BINARY);

    int* outImage = new int[w * h];
    for(int i=0;i<w*h;i++) {
        outImage[i]=(int)g_pBinaryImage->imageData[i];
    }

    int size = w * h;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, outImage);
    env->ReleaseIntArrayElements(buf, cbuf, 0);
    return result;
}

jstring str2jstring(JNIEnv* env, const char* pat) {
    //定义java String类 strClass
    jclass strClass = (env)->FindClass("Ljava/lang/String;");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = (env)->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = (env)->NewByteArray(strlen(pat));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = (env)->NewStringUTF("GB2312");
    //将byte数组转换为java String,并输出
    return (jstring)(env)->NewObject(strClass, ctorID, bytes, encoding);
}

std::string jstring2str(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr= (jbyteArray)env->CallObjectMethod(jstr,mid,strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr,JNI_FALSE);
    if(alen > 0) {
        rtn = (char*)malloc(alen+1);
        memcpy(rtn,ba,alen);
        rtn[alen]=0;
    }
    env->ReleaseByteArrayElements(barr,ba,0);
    std::string stemp(rtn);
    free(rtn);
    return stemp;
}

const char* string2printf(JNIEnv *env, std::string str) {
    jstring jstr = env->NewStringUTF(str.c_str());
    return env->GetStringUTFChars(jstr, NULL);
}

JNIEXPORT void JNICALL Java_com_lichao_opencv4androidface_uitls_JNIUtils_testParseJson
        (JNIEnv *env, jclass obj, jstring str) {
    LOGE("testParseJson Start");

    const char* json_str = env->GetStringUTFChars(str, NULL);
    LOGE("printf %s", json_str);

    std::string strValue = jstring2str(env, str);
    Json::Reader reader;
    Json::Value value;
    if (reader.parse(strValue, value)) {
        const Json::Value arrayObj = value["landmark"];
        for (int i=0; i<arrayObj.size(); i++) {
            std::string x = arrayObj[i]["x"].asString();
            std::string y = arrayObj[i]["y"].asString();
            std::string output = std::string("landmark ") + std::string("x=") + x + std::string(", y=") + y;
            jstring jstr = env->NewStringUTF(output.c_str());
            const char* json_str = env->GetStringUTFChars(jstr, NULL);
            LOGE("printf %s", json_str);
        }
    }

    LOGE("testParseJson End");
}

// Compute similarity transform given two pairs of corresponding points.
// OpenCV requires 3 pairs of corresponding points.
// We are faking the third one.
void similarityTransform(std::vector<cv::Point2f>& inPoints, std::vector<cv::Point2f>& outPoints, cv::Mat &tform) {
    double s60 = sin(60 * M_PI / 180.0);
    double c60 = cos(60 * M_PI / 180.0);

    vector <Point2f> inPts = inPoints;
    vector <Point2f> outPts = outPoints;

    inPts.push_back(cv::Point2f(0,0));
    outPts.push_back(cv::Point2f(0,0));


    inPts[2].x =  c60 * (inPts[0].x - inPts[1].x) - s60 * (inPts[0].y - inPts[1].y) + inPts[1].x;
    inPts[2].y =  s60 * (inPts[0].x - inPts[1].x) + c60 * (inPts[0].y - inPts[1].y) + inPts[1].y;

    outPts[2].x =  c60 * (outPts[0].x - outPts[1].x) - s60 * (outPts[0].y - outPts[1].y) + outPts[1].x;
    outPts[2].y =  s60 * (outPts[0].x - outPts[1].x) + c60 * (outPts[0].y - outPts[1].y) + outPts[1].y;


    tform = cv::estimateRigidTransform(inPts, outPts, false);
}

// Calculate Delaunay triangles for set of points
// Returns the vector of indices of 3 points for each triangle
static void calculateDelaunayTriangles(Rect rect, vector<Point2f> &points, vector< vector<int> > &delaunayTri){

    // Create an instance of Subdiv2D
    Subdiv2D subdiv(rect);

    // Insert points into subdiv
    for( vector<Point2f>::iterator it = points.begin(); it != points.end(); it++)
        subdiv.insert(*it);

    vector<Vec6f> triangleList;
    subdiv.getTriangleList(triangleList);
    vector<Point2f> pt(3);
    vector<int> ind(3);

    for( size_t i = 0; i < triangleList.size(); i++ )
    {
        Vec6f t = triangleList[i];
        pt[0] = Point2f(t[0], t[1]);
        pt[1] = Point2f(t[2], t[3]);
        pt[2] = Point2f(t[4], t[5 ]);

        if ( rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2])){
            for(int j = 0; j < 3; j++)
                for(size_t k = 0; k < points.size(); k++)
                    if(abs(pt[j].x - points[k].x) < 1.0 && abs(pt[j].y - points[k].y) < 1)
                        ind[j] = k;

            delaunayTri.push_back(ind);
        }
    }
}

// Apply affine transform calculated using srcTri and dstTri to src
void applyAffineTransform(Mat &warpImage, Mat &src, vector<Point2f> &srcTri, vector<Point2f> &dstTri) {
    // Given a pair of triangles, find the affine transform.
    Mat warpMat = getAffineTransform( srcTri, dstTri );

    // Apply the Affine Transform just found to the src image
    warpAffine( src, warpImage, warpMat, warpImage.size(), INTER_LINEAR, BORDER_REFLECT_101);
}


// Warps and alpha blends triangular regions from img1 and img2 to img
void warpTriangle(Mat &img1, Mat &img2, vector<Point2f> t1, vector<Point2f> t2) {
    // Find bounding rectangle for each triangle
    Rect r1 = boundingRect(t1);
    Rect r2 = boundingRect(t2);

    // Offset points by left top corner of the respective rectangles
    vector<Point2f> t1Rect, t2Rect;
    vector<Point> t2RectInt;
    for(int i = 0; i < 3; i++)
    {
        //tRect.push_back( Point2f( t[i].x - r.x, t[i].y -  r.y) );
        t2RectInt.push_back( Point((int)(t2[i].x - r2.x), (int)(t2[i].y - r2.y)) ); // for fillConvexPoly

        t1Rect.push_back( Point2f( t1[i].x - r1.x, t1[i].y -  r1.y) );
        t2Rect.push_back( Point2f( t2[i].x - r2.x, t2[i].y - r2.y) );
    }

    // Get mask by filling triangle
    Mat mask = Mat::zeros(r2.height, r2.width, CV_32FC3);
    fillConvexPoly(mask, t2RectInt, Scalar(1.0, 1.0, 1.0), 16, 0);

    // Apply warpImage to small rectangular patches
    Mat img1Rect, img2Rect;
    img1(r1).copyTo(img1Rect);

    Mat warpImage = Mat::zeros(r2.height, r2.width, img1Rect.type());

    applyAffineTransform(warpImage, img1Rect, t1Rect, t2Rect);

    // Copy triangular region of the rectangular patch to the output image
    multiply(warpImage,mask, warpImage);
    multiply(img2(r2), Scalar(1.0,1.0,1.0) - mask, img2(r2));
    img2(r2) = img2(r2) + warpImage;
}

// Constrains points to be inside boundary
void constrainPoint(Point2f &p, Size sz) {
    p.x = min(max( (double)p.x, 0.0), (double)(sz.width - 1));
    p.y = min(max( (double)p.y, 0.0), (double)(sz.height - 1));
}

// Read points from list of text file
void readPoints(vector<string> pointsFileNames, vector<vector<Point2f> > &pointsVec) {

    for(size_t i = 0; i < pointsFileNames.size(); i++) {
        vector<Point2f> points;
        ifstream ifs(pointsFileNames[i].c_str());
        float x, y;
        while(ifs >> x >> y)
            points.push_back(Point2f((float)x, (float)y));

        pointsVec.push_back(points);
    }
}

// Read names from the directory
void readFileNames(JNIEnv *env, string dirName, vector<string> &imageFnames, vector<string> &ptsFnames) {
    DIR *dir;
    struct dirent *ent;
    int count = 0;

    //image extensions
    string imgExt = "jpg";
    string imgExt2 = "png";
    string txtExt = "txt";

    if ((dir = opendir (dirName.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if(count < 2) {
                count++;
                continue;
            }

            string path = dirName;
            string fname = ent->d_name;

            if (fname.find(imgExt, (fname.length() - imgExt.length())) != std::string::npos) {
                path.append(fname);
                imageFnames.push_back(path);
            }
            else if (fname.find(imgExt2, (fname.length() - imgExt2.length())) != std::string::npos) {
                path.append(fname);
                imageFnames.push_back(path);
            }
            else if (fname.find(txtExt, (fname.length() - txtExt.length())) != std::string::npos) {
                path.append(fname);
                ptsFnames.push_back(path);
            }
            LOGE("printf path %s", string2printf(env, path));  // TODO
        }
        closedir (dir);
    }
}

JNIEXPORT void JNICALL Java_com_simoncherry_averageface_JNIUtils_testReadFile
        (JNIEnv *env, jclass obj, jstring path) {
    const char* path_str = env->GetStringUTFChars(path, NULL);
    LOGE("printf path %s", path_str);

    string dirName = jstring2str(env, path);

    vector<string> imageNames, ptsNames;
    readFileNames(env, dirName, imageNames, ptsNames);

    //if(imageNames.empty() || ptsNames.empty() || imageNames.size() != ptsNames.size()) {
    if(ptsNames.empty()) {
        //LOGE("读到的图片为空 or 关键点文本为空 or 图片与文本数量不一致");
        LOGE("读到的关键点文本为空");
    } else {
        for (int i=0; i<ptsNames.size(); i++) {
            LOGE("printf ptsNames %s", string2printf(env, ptsNames[i]));
        }

        vector<vector<Point2f> > allPoints;
        readPoints(ptsNames, allPoints);
    }
}

/*
 * Class:     com_simoncherry_averageface_JNIUtils
 * Method:    averageFaceTest 平均脸生成测试，path为图片目录，取目录所有图片进行合成
 * Signature: (Ljava/lang/String;)[I
 */
JNIEXPORT jstring JNICALL Java_com_lichao_opencv4androidface_uitls_JNIUtils_averageFaceTest
        (JNIEnv *env, jclass obj, jstring path) {

    LOGE("averageFaceTest Start");
    // Dimensions of output image
    int w = 300;
    int h = 300;

    // Read images in the directory
    vector<string> imageNames, ptsNames;
    string dirName = jstring2str(env, path);
    readFileNames(env, dirName, imageNames, ptsNames);

    if(imageNames.empty() || ptsNames.empty() || imageNames.size() != ptsNames.size()) {
        return NULL;
    }

    // Read points
    vector<vector<Point2f> > allPoints;
    readPoints(ptsNames, allPoints);

    int n = allPoints[0].size();

    // Read images
    vector<Mat> images;
    for(size_t i = 0; i < imageNames.size(); i++){
        Mat img = imread(imageNames[i]);

        img.convertTo(img, CV_32FC3, 1/255.0);

        if(!img.data) {
            //cout << "image " << imageNames[i] << " not read properly" << endl;
        } else {
            images.push_back(img);
        }
    }

    if(images.empty()) {
        return NULL;
    }

    int numImages = images.size();

    // Eye corners
    vector<Point2f> eyecornerDst, eyecornerSrc;
    eyecornerDst.push_back(Point2f( 0.3*w, h/3));
    eyecornerDst.push_back(Point2f( 0.7*w, h/3));

    eyecornerSrc.push_back(Point2f(0,0));
    eyecornerSrc.push_back(Point2f(0,0));

    // Space for normalized images and points.
    vector <Mat> imagesNorm;
    vector < vector <Point2f> > pointsNorm;

    // Space for average landmark points
    vector <Point2f> pointsAvg(allPoints[0].size());

    // 8 Boundary points for Delaunay Triangulation
    vector <Point2f> boundaryPts;
    boundaryPts.push_back(Point2f(0,0));
    boundaryPts.push_back(Point2f(w/2, 0));
    boundaryPts.push_back(Point2f(w-1,0));
    boundaryPts.push_back(Point2f(w-1, h/2));
    boundaryPts.push_back(Point2f(w-1, h-1));
    boundaryPts.push_back(Point2f(w/2, h-1));
    boundaryPts.push_back(Point2f(0, h-1));
    boundaryPts.push_back(Point2f(0, h/2));

    // Warp images and trasnform landmarks to output coordinate system,
    // and find average of transformed landmarks.

    for(size_t i = 0; i < images.size(); i++) {
        vector <Point2f> points = allPoints[i];

        // The corners of the eyes are the landmarks number 36 and 45
        eyecornerSrc[0] = allPoints[i][36];
        eyecornerSrc[1] = allPoints[i][45];

        // Calculate similarity transform
        Mat tform;
        similarityTransform(eyecornerSrc, eyecornerDst, tform);

        // Apply similarity transform to input image and landmarks
        Mat img = Mat::zeros(h, w, CV_32FC3);
        warpAffine(images[i], img, tform, img.size());
        transform( points, points, tform);

        // Calculate average landmark locations
        for ( size_t j = 0; j < points.size(); j++){
            pointsAvg[j] += points[j] * ( 1.0 / numImages);
        }

        // Append boundary points. Will be used in Delaunay Triangulation
        for ( size_t j = 0; j < boundaryPts.size(); j++) {
            points.push_back(boundaryPts[j]);
        }

        pointsNorm.push_back(points);
        imagesNorm.push_back(img);
    }

    // Append boundary points to average points.
    for ( size_t j = 0; j < boundaryPts.size(); j++){
        pointsAvg.push_back(boundaryPts[j]);
    }

    // Calculate Delaunay triangles
    Rect rect(0, 0, w, h);
    vector< vector<int> > dt;
    calculateDelaunayTriangles(rect, pointsAvg, dt);

    // Space for output image
    Mat output_mat = Mat::zeros(h, w, CV_32FC3);
    Size size(w,h);

    // Warp input images to average image landmarks
    for(size_t i = 0; i < numImages; i++) {
        Mat img = Mat::zeros(h, w, CV_32FC3);
        // Transform triangles one by one
        for(size_t j = 0; j < dt.size(); j++) {
            // Input and output points corresponding to jth triangle
            vector<Point2f> tin, tout;
            for(int k = 0; k < 3; k++) {
                Point2f pIn = pointsNorm[i][dt[j][k]];
                constrainPoint(pIn, size);

                Point2f pOut = pointsAvg[dt[j][k]];
                constrainPoint(pOut,size);

                tin.push_back(pIn);
                tout.push_back(pOut);
            }

            warpTriangle(imagesNorm[i], img, tin, tout);
        }
        // Add image intensities for averaging
        output_mat = output_mat + img;
    }

    // Divide by numImages to get average
    output_mat = output_mat / (double)numImages;  // Mat output_mat = Mat::zeros(h, w, CV_32FC3);

    output_mat.convertTo(output_mat, CV_8UC3, 255.0);
    vector<int> parameters;
    parameters.push_back(CV_IMWRITE_JPEG_QUALITY);
    parameters.push_back(100);
    std::string result_path = "/sdcard/average_face_result.jpg";

//    std::string md5_path = MD5(result_path).toStr();
//    LOGE("printf md5 %s", string2printf(env, md5_path));

    imwrite(result_path, output_mat, parameters);
    LOGE("averageFaceTest End");
    return env->NewStringUTF(result_path.c_str());
}

/*
 * Class:     com_simoncherry_averageface_JNIUtils
 * Method:    doAverageFace 根据stringArray的图片路径合成平均脸
 * Signature: ([Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_lichao_opencv4androidface_uitls_JNIUtils_doAverageFace
        (JNIEnv *env, jclass obj, jobjectArray stringArray) {

    LOGE("doAverageFace Start");
    // Dimensions of output image
    int w = 300;
    int h = 300;

    vector<string> imageNames, ptsNames;

    int stringCount = env->GetArrayLength(stringArray);
    for (int i=0; i<stringCount; i++) {
        jstring prompt = (jstring) (env->GetObjectArrayElement(stringArray, i));
        std::string img_path = jstring2str(env, prompt);
        imageNames.push_back(img_path);

        const char *rawString = env->GetStringUTFChars(prompt, 0);
        LOGE("printf path %s", rawString);
        // Don't forget to call `ReleaseStringUTFChars` when you're done.
        env->ReleaseStringUTFChars(prompt, rawString);

        std::string txt_path = MD5(img_path).toStr();
        txt_path = "/data/data/com.simoncherry.averageface/files/" + txt_path + ".txt";
        ptsNames.push_back(txt_path);
        LOGE("printf md5 %s", string2printf(env, txt_path));
    }

    if(imageNames.empty() || ptsNames.empty() || imageNames.size() != ptsNames.size()) {
        return NULL;
    }

    // Read points
    vector<vector<Point2f> > allPoints;
    readPoints(ptsNames, allPoints);

    int n = allPoints[0].size();
    // Read images
    vector<Mat> images;
    for(size_t i = 0; i < imageNames.size(); i++){
        Mat img = imread(imageNames[i]);

        img.convertTo(img, CV_32FC3, 1/255.0);

        if(!img.data) {
            //cout << "image " << imageNames[i] << " not read properly" << endl;
        } else {
            images.push_back(img);
        }
    }

    if(images.empty()) {
        return NULL;
    }

    int numImages = images.size();

    // Eye corners
    vector<Point2f> eyecornerDst, eyecornerSrc;
    eyecornerDst.push_back(Point2f( 0.3*w, h/3));
    eyecornerDst.push_back(Point2f( 0.7*w, h/3));

    eyecornerSrc.push_back(Point2f(0,0));
    eyecornerSrc.push_back(Point2f(0,0));

    // Space for normalized images and points.
    vector <Mat> imagesNorm;
    vector < vector <Point2f> > pointsNorm;

    // Space for average landmark points
    vector <Point2f> pointsAvg(allPoints[0].size());

    // 8 Boundary points for Delaunay Triangulation
    vector <Point2f> boundaryPts;
    boundaryPts.push_back(Point2f(0,0));
    boundaryPts.push_back(Point2f(w/2, 0));
    boundaryPts.push_back(Point2f(w-1,0));
    boundaryPts.push_back(Point2f(w-1, h/2));
    boundaryPts.push_back(Point2f(w-1, h-1));
    boundaryPts.push_back(Point2f(w/2, h-1));
    boundaryPts.push_back(Point2f(0, h-1));
    boundaryPts.push_back(Point2f(0, h/2));

    // Warp images and trasnform landmarks to output coordinate system,
    // and find average of transformed landmarks.

    for(size_t i = 0; i < images.size(); i++) {
        vector <Point2f> points = allPoints[i];

        // The corners of the eyes are the landmarks number 36 and 45
        eyecornerSrc[0] = allPoints[i][36];
        eyecornerSrc[1] = allPoints[i][45];

        // Calculate similarity transform
        Mat tform;
        similarityTransform(eyecornerSrc, eyecornerDst, tform);

        // Apply similarity transform to input image and landmarks
        Mat img = Mat::zeros(h, w, CV_32FC3);
        warpAffine(images[i], img, tform, img.size());
        transform( points, points, tform);

        // Calculate average landmark locations
        for ( size_t j = 0; j < points.size(); j++){
            pointsAvg[j] += points[j] * ( 1.0 / numImages);
        }

        // Append boundary points. Will be used in Delaunay Triangulation
        for ( size_t j = 0; j < boundaryPts.size(); j++) {
            points.push_back(boundaryPts[j]);
        }

        pointsNorm.push_back(points);
        imagesNorm.push_back(img);
    }

    // Append boundary points to average points.
    for ( size_t j = 0; j < boundaryPts.size(); j++){
        pointsAvg.push_back(boundaryPts[j]);
    }

    // Calculate Delaunay triangles
    Rect rect(0, 0, w, h);
    vector< vector<int> > dt;
    calculateDelaunayTriangles(rect, pointsAvg, dt);

    // Space for output image
    Mat output_mat = Mat::zeros(h, w, CV_32FC3);
    Size size(w,h);

    // Warp input images to average image landmarks
    for(size_t i = 0; i < numImages; i++) {
        Mat img = Mat::zeros(h, w, CV_32FC3);
        // Transform triangles one by one
        for(size_t j = 0; j < dt.size(); j++) {
            // Input and output points corresponding to jth triangle
            vector<Point2f> tin, tout;
            for(int k = 0; k < 3; k++) {
                Point2f pIn = pointsNorm[i][dt[j][k]];
                constrainPoint(pIn, size);

                Point2f pOut = pointsAvg[dt[j][k]];
                constrainPoint(pOut,size);

                tin.push_back(pIn);
                tout.push_back(pOut);
            }

            warpTriangle(imagesNorm[i], img, tin, tout);
        }
        // Add image intensities for averaging
        output_mat = output_mat + img;
    }

    // Divide by numImages to get average
    output_mat = output_mat / (double)numImages;  // Mat output_mat = Mat::zeros(h, w, CV_32FC3);

    output_mat.convertTo(output_mat, CV_8UC3, 255.0);
    vector<int> parameters;
    parameters.push_back(CV_IMWRITE_JPEG_QUALITY);
    parameters.push_back(100);
    std::string result_path = "/sdcard/average_face_result.jpg";
    imwrite(result_path, output_mat, parameters);
    LOGE("doAverageFace End");
    return env->NewStringUTF(result_path.c_str());
}

/*
 * Class:     com_simoncherry_averageface_JNIUtils
 * Method:    doFaceSwap
 * Signature: ([Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_lichao_opencv4androidface_uitls_JNIUtils_oFaceSwap
        (JNIEnv *env, jclass obj, jobjectArray stringArray) {
    LOGE("doFaceSwap Start");
    int stringCount = env->GetArrayLength(stringArray);
    if (stringCount < 2) {
        return NULL;
    }

    vector<string> imageNames, ptsNames;

    for (int i=0; i<2; i++) {
        jstring prompt = (jstring) (env->GetObjectArrayElement(stringArray, i));
        std::string img_path = jstring2str(env, prompt);
        imageNames.push_back(img_path);

        const char *rawString = env->GetStringUTFChars(prompt, 0);
        LOGE("printf path %s", rawString);
        // Don't forget to call `ReleaseStringUTFChars` when you're done.
        env->ReleaseStringUTFChars(prompt, rawString);

        std::string txt_path = MD5(img_path).toStr();
        txt_path = "/data/data/com.simoncherry.averageface/files/" + txt_path + ".txt";
        ptsNames.push_back(txt_path);
        LOGE("printf md5 %s", string2printf(env, txt_path));
    }

    if(imageNames.empty() || ptsNames.empty() || imageNames.size() != ptsNames.size()) {
        return NULL;
    }

    // Read points
    vector<vector<Point2f> > allPoints;
    readPoints(ptsNames, allPoints);

    vector<Point2f> points1 = allPoints[0];
    vector<Point2f> points2 = allPoints[1];

    Mat img1 = imread(imageNames[0]);
    Mat img2 = imread(imageNames[1]);
    Mat img1Warped = img2.clone();

    //convert Mat to float data type
    img1.convertTo(img1, CV_32F);
    img1Warped.convertTo(img1Warped, CV_32F);

    // Find convex hull
    vector<Point2f> hull1;
    vector<Point2f> hull2;
    vector<int> hullIndex;

    convexHull(points2, hullIndex, false, false);

    for(int i = 0; i < hullIndex.size(); i++)
    {
        hull1.push_back(points1[hullIndex[i]]);
        hull2.push_back(points2[hullIndex[i]]);
    }

    // Find delaunay triangulation for points on the convex hull
    vector< vector<int> > dt;
    Rect rect(0, 0, img1Warped.cols, img1Warped.rows);
    calculateDelaunayTriangles(rect, hull2, dt);

    // Apply affine transformation to Delaunay triangles
    for(size_t i = 0; i < dt.size(); i++)
    {
        vector<Point2f> t1, t2;
        // Get points for img1, img2 corresponding to the triangles
        for(size_t j = 0; j < 3; j++)
        {
            t1.push_back(hull1[dt[i][j]]);
            t2.push_back(hull2[dt[i][j]]);
        }

        warpTriangle(img1, img1Warped, t1, t2);
    }

    // Calculate mask
    vector<Point> hull8U;
    for(int i = 0; i < hull2.size(); i++)
    {
        Point pt(hull2[i].x, hull2[i].y);
        hull8U.push_back(pt);
    }

    Mat mask = Mat::zeros(img2.rows, img2.cols, img2.depth());
    fillConvexPoly(mask,&hull8U[0], hull8U.size(), Scalar(255,255,255));

    // Clone seamlessly.
    Rect r = boundingRect(hull2);
    Point center = (r.tl() + r.br()) / 2;

    Mat output;
    img1Warped.convertTo(img1Warped, CV_8UC3);
    seamlessClone(img1Warped,img2, mask, center, output, NORMAL_CLONE);

    vector<int> parameters;
    parameters.push_back(CV_IMWRITE_JPEG_QUALITY);
    parameters.push_back(100);
    std::string result_path = "/sdcard/face_swap_result.jpg";
    imwrite(result_path, output, parameters);
    LOGE("doAverageFace End");
    return env->NewStringUTF(result_path.c_str());
}

std::vector<cv::Point3d> get_3d_model_points()
{
    std::vector<cv::Point3d> modelPoints;

    modelPoints.push_back(cv::Point3d(0.0f, 0.0f, 0.0f)); //The first must be (0,0,0) while using POSIT
    modelPoints.push_back(cv::Point3d(0.0f, -330.0f, -65.0f));
    modelPoints.push_back(cv::Point3d(-225.0f, 170.0f, -135.0f));
    modelPoints.push_back(cv::Point3d(225.0f, 170.0f, -135.0f));
    modelPoints.push_back(cv::Point3d(-150.0f, -150.0f, -125.0f));
    modelPoints.push_back(cv::Point3d(150.0f, -150.0f, -125.0f));

    return modelPoints;

}

std::vector<cv::Point2d> get_2d_image_points(vector<Point2f> points)
{
    std::vector<cv::Point2d> image_points;
    image_points.push_back( cv::Point2d( points[30].x, points[30].y ) );    // Nose tip
    image_points.push_back( cv::Point2d( points[8].x, points[8].y ) );      // Chin
    image_points.push_back( cv::Point2d( points[36].x, points[36].y ) );    // Left eye left corner
    image_points.push_back( cv::Point2d( points[45].x, points[45].y ) );    // Right eye right corner
    image_points.push_back( cv::Point2d( points[48].x, points[48].y ) );    // Left Mouth corner
    image_points.push_back( cv::Point2d( points[54].x, points[54].y ) );    // Right mouth corner
    return image_points;

}

cv::Mat get_camera_matrix(float focal_length, cv::Point2d center)
{
    cv::Mat camera_matrix = (cv::Mat_<double>(3,3) << focal_length, 0, center.x, 0 , focal_length, center.y, 0, 0, 1);
    return camera_matrix;
}

/*
 * Class:     com_simoncherry_averageface_JNIUtils
 * Method:    showHeadPose 画出头部姿态的三轴
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_lichao_opencv4androidface_uitls_JNIUtils_showHeadPose
        (JNIEnv *env, jclass obj, jstring path) {
    LOGE("Head Pose START");

    string img_path = jstring2str(env, path);
    string txt_path = MD5(img_path).toStr();
    txt_path = "/data/data/com.simoncherry.averageface/files/" + txt_path + ".txt";

    vector<Point2f> points;
    ifstream ifs(txt_path.c_str());
    float x, y;
    while(ifs >> x >> y)
        points.push_back(Point2f((float)x, (float)y));

    Mat img = imread(img_path);
    // 检测关键点时缩小了图片，所以这里也要按相同的方式缩小图片
    float max_size = 512;
    if (img.cols > max_size && img.rows > max_size) {
        float ratio = img.cols / (float)img.rows;
        Size dsize = Size(max_size, round(max_size / ratio));
        resize(img, img, dsize);
    }

    // 画关键点
    for (size_t i = 0; i < points.size(); i++){
        cv::Point2f point =  cv::Point(points[i].x, points[i].y);
        cv::circle(img, point, 3, cv::Scalar(0, 255, 255), -1);
    }

    // 推算Head Pose
    std::vector<cv::Point3d> model_points = get_3d_model_points();
    std::vector<cv::Point2d> image_points = get_2d_image_points(points);
    double focal_length = img.cols;
    cv::Mat camera_matrix = get_camera_matrix(focal_length, cv::Point2d(img.cols/2,img.rows/2));
    cv::Mat rotation_vector;
    cv::Mat rotation_matrix;
    cv::Mat translation_vector;

    cv::Mat dist_coeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type);
    cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);

    std::vector<cv::Point3d> nose_end_point3D;
    std::vector<cv::Point2d> nose_end_point2D;
    nose_end_point3D.push_back(cv::Point3d(0, 0, 400.0));
    nose_end_point3D.push_back(cv::Point3d(0, 400.0, 0));
    nose_end_point3D.push_back(cv::Point3d(400.0, 0, 0));

    cv::projectPoints(nose_end_point3D, rotation_vector, translation_vector, camera_matrix, dist_coeffs, nose_end_point2D);
    // 画x、y、z轴
    cv::line(img, image_points[0], nose_end_point2D[0], cv::Scalar(0, 0, 255), 2);
    cv::line(img, image_points[0], nose_end_point2D[1], cv::Scalar(0, 255, 0), 2);
    cv::line(img, image_points[0], nose_end_point2D[2], cv::Scalar(255, 0, 0), 2);

    vector<int> parameters;
    parameters.push_back(CV_IMWRITE_JPEG_QUALITY);
    parameters.push_back(100);
    std::string result_path = "/sdcard/head_pose_result.jpg";
    imwrite(result_path, img, parameters);
    LOGE("Head Pose End");
    return env->NewStringUTF(result_path.c_str());
}

#ifdef __cplusplus
}
#endif
