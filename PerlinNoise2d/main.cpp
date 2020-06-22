/*研究2维柏林噪声*/

#include <opencv2/opencv.hpp>

float PNoise2dUnit(float dx, float dy,
    float g0x, float g0y, float g1x, float g1y,
    float g2x, float g2y, float g3x, float g3y)
{
    float dx1 = dx - 1;
    float dy1 = dy - 1;

    
    float i0 = dx * g0x + dy * g0y;
    float i1 = dx1 * g1x + dy * g1y;
    float i2 = dx * g2x + dy1 * g2y;
    float i3 = dx1 * g3x + dy1 * g3y;

    float fade_dx = dx * dx * dx * (6 * dx * dx - 15 * dx + 10);
    float i01 = i0 + fade_dx * (i1 - i0);
    float i23 = i2 + fade_dx * (i3 - i2);
    float i = i01 + dy * dy * dy * (6 * dy * dy - 15 * dy + 10) * (i23 - i01);

    return i;
}

void constPNoise2d(int refLen,
    const std::vector<std::vector<std::pair<float, float>>>& gmat,
    std::vector<std::vector<float>>& noiseMat)
{
    int refh = int(gmat.size());
    int refw = int(gmat[0].size());

    int noiseh = refh * refLen;
    int noisew = refw * refLen;

    noiseMat = std::vector<std::vector<float>>(noiseh, std::vector<float>(noisew));
    for (int i = 0; i < noiseh; i++) {
        for (int j = 0; j < noisew; j++) {
            float dx = (j % refLen) / float(refLen);
            float dy = (i % refLen) / float(refLen);

            int b0x = j / refLen;
            int b0y = i / refLen;
            int b1x = b0x + 1;
            int b1y = b0y + 1;
            if (b1x == refw) b1x = 0;
            if (b1y == refh) b1y = 0;

            float g0x = gmat[b0y][b0x].first;
            float g0y = gmat[b0y][b0x].second;
            float g1x = gmat[b0y][b1x].first;
            float g1y = gmat[b0y][b1x].second;
            float g2x = gmat[b1y][b0x].first;
            float g2y = gmat[b1y][b0x].second;
            float g3x = gmat[b1y][b1x].first;
            float g3y = gmat[b1y][b1x].second;

            noiseMat[i][j] = PNoise2dUnit(dx, dy, g0x, g0y, g1x, g1y, g2x, g2y, g3x, g3y);
        }
    }
}

/*

const std::string WINDOWNAME = "canvas";

//全局变量gmat
std::vector<std::vector<std::pair<float, float>>> gmat;

int refLen;

cv::Mat canvas;

void vizNoise(int refLen, const std::vector<std::vector<std::pair<float, float>>>& gmat)
{
    //生成噪声
    std::vector<std::vector<float>> noiseMat;
    constPNoise2d(refLen, gmat, noiseMat);

    //创建画布
    int h = gmat.size() * refLen;
    int w = gmat[0].size() * refLen;
    canvas = cv::Mat(h + refLen, w + refLen, CV_8UC3);
    canvas = cv::Scalar(255, 0, 0);

    //绘制
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            float noise = noiseMat[i][j];

            int pixv = noise * 127 + 127;

            canvas.at<cv::Vec3b>(i+refLen,j+refLen) = cv::Vec3b(pixv,pixv,pixv);
        }
    }

    for (int i = 0; i < gmat.size(); i++) {
        cv::line(canvas, cv::Point(refLen, i * refLen + refLen), cv::Point(w - 1 + refLen, i * refLen + refLen), cv::Scalar(0, 0, 255));
    }
    for (int j = 0; j < gmat[0].size(); j++) {
        cv::line(canvas, cv::Point(j * refLen + refLen, refLen), cv::Point(j * refLen + refLen, h - 1 + refLen), cv::Scalar(0, 0, 255));
    }

    for (int i = 0; i < gmat.size(); i++) {
        for (int j = 0; j < gmat[i].size(); j++) {
            float gx = gmat[i][j].first;
            float gy = gmat[i][j].second;

            int x0 = j * refLen + refLen;
            int y0 = i * refLen + refLen;
            int x1 = x0 + gx * refLen;
            int y1 = y0 + gy * refLen;

            cv::line(canvas, cv::Point(x0, y0), cv::Point(x1, y1), cv::Scalar(0, 255, 0));
        }
    }

    //显示
    cv::namedWindow(WINDOWNAME, 0);
    cv::imshow(WINDOWNAME, canvas);
}

void onMouse(int event, int x, int y, int flags, void* param)
{
    if (event == CV_EVENT_LBUTTONDOWN)//鼠标左键按下事件
    {
        //显示坐标和像素值
        std::cout << "At (" << x << "," << y << ") value is :"
            << static_cast<int>(canvas.at<uchar>(y, x)) << std::endl;

        float dx = 0;
        float dy = 0;

        x -= refLen;
        y -= refLen;

        if (x % refLen > refLen / 2) {
            dx = x - (x / refLen * refLen + refLen);
            x = x / refLen * refLen + refLen;
        }
        else {
            dx = x - x / refLen * refLen;
            x = x / refLen * refLen;
        }

        if (y % refLen > refLen / 2) {
            dy = y - (y / refLen * refLen + refLen);
            y = y / refLen * refLen + refLen;
        }
        else {
            dy = y - y / refLen * refLen;
            y = y / refLen * refLen;
        }

        dx /= float(refLen);
        dy /= float(refLen);

        int i = y / refLen;
        int j = x / refLen;

        gmat[i][j] = std::make_pair(dx, dy);
        std::cout << gmat[i][j].first << std::endl;
        std::cout << gmat[i][j].second << std::endl;

        vizNoise(refLen, gmat);
    }
}

int main()
{
    //尺寸
    refLen = 16;
    int w = 128;
    int h = 128;
    int gmatw = w / refLen;
    int gmath = h / refLen;

    //为gmat分配空间
    gmat = std::vector<std::vector<std::pair<float, float>>>(gmath, std::vector<std::pair<float, float>>(gmatw));

    //gmat随机初始化
    srand(123);
    for (int i = 0; i < gmath; i++) {
        for (int j = 0; j < gmatw; j++) {
            gmat[i][j].first = (rand() % 100) / float(50) - 1;
            gmat[i][j].second = (rand() % 100) / float(50) - 1;
        }
    }

    //初次显示
    vizNoise(refLen, gmat);

    //注册回调函数
    cv::setMouseCallback(WINDOWNAME, onMouse, nullptr);

    cv::waitKey();

    return 0;
}

*/

void genRandNoise(int rdseed, int w, int h, int refLen, 
    std::vector<std::vector<float>>& noiseMat)
{
    std::vector<std::vector<std::pair<float, float>>> gmat;

    int gmatw = w / refLen;
    int gmath = h / refLen;

    //为gmat分配空间
    gmat = std::vector<std::vector<std::pair<float, float>>>(gmath, std::vector<std::pair<float, float>>(gmatw));

    srand(rdseed);
    for (int i = 0; i < gmath; i++) {
        for (int j = 0; j < gmatw; j++) {
            gmat[i][j].first = (rand() % 100) / float(50) - 1;
            gmat[i][j].second = (rand() % 100) / float(50) - 1;
        }
    }

    constPNoise2d(refLen, gmat, noiseMat);
}

int main()
{
    int w = 256;
    int h = 256;
    int refLen = 32;

    int rdseed = 123;
    std::vector<std::vector<float>> noiseMat1;
    std::vector<std::vector<float>> noiseMat2;
    genRandNoise(rdseed, w, h, refLen, noiseMat1);
    genRandNoise(rdseed+1, w, h, refLen, noiseMat2);

    cv::Mat canvas(h, w, CV_8UC3);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            cv::Vec3b pixclr(0,0,0);

            float noise1 = noiseMat1[i][j];
            float noise2 = noiseMat2[i][j];

            /*
            if (noise1 > 0 && noise2 > 0)
                pixclr = cv::Vec3b(255, 0, 0);
            else if (noise1 > 0 && noise2 <= 0)
                pixclr = cv::Vec3b(0, 255, 0);
            else if (noise1 <= 0 && noise2 > 0)
                pixclr = cv::Vec3b(0, 0, 255);
            else
                pixclr = cv::Vec3b(255, 255, 0);
            */

            //float noise = std::sin(10*noise2*noise1);
            
            if (noise2 == 0) noise2 = 1e-5;
            float noise = std::atan2(noise1,noise2);

            if (noise < 3.14/4.0f && noise > -3.14/4.0f)
                pixclr = cv::Vec3b(255, 0, 0);
            else if (noise < 3.14*3/4.0f && noise > 3.14/4.0f)
                pixclr = cv::Vec3b(0, 255, 0);
            else if (noise < -3.14/4.0f && noise > -3.14*3/4.0f)
                pixclr = cv::Vec3b(0, 0, 255);
            else
                pixclr = cv::Vec3b(0, 150, 155);
            

            canvas.at<cv::Vec3b>(i, j) = pixclr;
        }
    }

    cv::namedWindow("canvas", 0);
    cv::imshow("canvas", canvas);
    cv::waitKey();

    return 0;
}