/*
 * Copyright (C) 2013 EFAA Consortium, European Commission FP7 Project IST-270490
 * Authors: Ilaria Gori, Sean Ryan Fanello
 * email:   ilaria.gori@iit.it, sean.fanello@iit.it
 * website: http://efaa.upf.edu/
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * $EFAA_ROOT/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#include <yarp/os/Network.h>

#include "HOGThread.h"
#include "sceneFlowThread.h"
#include "Recognizer.h"

#define VOCAB_CMD_ACK            VOCAB3('a','c','k')
#define VOCAB_CMD_NACK           VOCAB4('n','a','c','k')
#define VOCAB_CMD_REC            VOCAB3('r','e','c')
#define VOCAB_CMD_STOP           VOCAB4('s','t','o','p')
#define VOCAB_CMD_TRAIN          VOCAB4('t','r','a','i')
#define VOCAB_CMD_SAVE           VOCAB4('s','a','v','e')
#define VOCAB_CMD_WRITE          VOCAB4('w','r','i','t')
#define VOCAB_CMD_SET            VOCAB3('s','e','t')
#define VOCAB_CMD_UPDATE_INITIAL VOCAB3('p','o','s')
#define STATE_RECOGNITION        0
#define STATE_WRITE              1
#define STATE_DONOTHING          2

class GestRecognition: public yarp::os::RFModule
{
    DisparityThread* disp;
    OpticalFlowThread* opt;
    SceneFlowThread* sceneT;
    HOGThread* hogT;

    Port featureOutput;
    Port scoresInput;
    RpcClient rpcClassifier;

    Recognizer recognizer;
    yarp::os::Port rpc;
    yarp::os::Port out;
    yarp::os::Port outSpeak;

    yarp::os::Stamp TSLeft;
    yarp::os::Stamp TSRight;
    yarp::sig::ImageOf<yarp::sig::PixelRgb>* imageL;
    yarp::sig::ImageOf<yarp::sig::PixelRgb>* imageR;

    IplImage* imgLPrev;
    IplImage* imgRPrev;
    IplImage* imgLNext;
    IplImage* imgRNext;
    IplImage* outDisp;
    IplImage* outOpt;
    IplImage* maskNew;
    IplImage* initPosition;
    IplImage* frameDiff;
    IplImage* tmpMaskNew;
    Mat optFlow;
    Mat dispOld;
    Mat dispNew;

    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > imagePortInLeft;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > imagePortInRight;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelBgr> > outImage;

    yarp::os::Semaphore* mutex;

    yarp::sig::ImageOf<yarp::sig::PixelBgr> depthToDisplay;
    
    yarp::sig::Vector frameFeatures; 
    yarp::sig::Vector scores;
    yarp::sig::Vector flowFeatures;

    yarp::sig::Vector flowHist;
    yarp::sig::Vector hogHist;

    int frameFeatureSize;
    double sumInitialFrame;
    double sumFrameDiff;
    std::deque<yarp::sig::Vector> featuresBuffer;

    std::string outDir;
    std::string context;
    int frameNumber;
    int bufferCount;
    int countFile;
    int threshold;
    int value;
    int width;
    int height;
    int state;
    ofstream featuresHOGFile;
    ofstream featuresHOFFile;
    int SVMBuffer;

    bool initL;
    bool initR;
    bool init;
    bool useDictionary;

private:
    void thresholdBW(IplImage *img, IplImage* res, int threshold, int x, int y, double value);
    void saveFeatures(ofstream &featuresFile, yarp::sig::Vector &Features);
    void retrieveOrderedScores(yarp::os::Bottle &class_scores);

public:

    GestRecognition();
    bool configure(yarp::os::ResourceFinder &rf);
    bool close();
    bool updateModule();
    double getPeriod();
    bool respond(const yarp::os::Bottle& cmd, yarp::os::Bottle& reply);
};
