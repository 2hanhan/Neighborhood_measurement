#include <iostream>
#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>

using namespace std;

typedef struct
{
    int index;
    int pixel;
} sort_st;
bool compare(sort_st a, sort_st b)
{
    return a.pixel < b.pixel; //降序
}

// step 1 计算邻域测度
cv::Mat Neighborhood_measurement(double k, cv::Mat image)
{
    cout << "邻域测度" << endl;
    cout << "锐化系数" << k << endl;
    cv::Mat image_mean9;
    cv::Mat Neighborhood_measurement_space;
    cv::blur(image, image_mean9, cv::Size(3, 3));
    cv::imshow("均值滤波", image_mean9);
    cv::waitKey(1000);
    Neighborhood_measurement_space = (9.0 / 8.0 * k + 1.0) * image - k * 9.0 / 8.0 * image_mean9;
    cv::imshow("邻域测度空间", Neighborhood_measurement_space);
    cv::waitKey(1000);
    return Neighborhood_measurement_space;
}

// step 2 排序
vector<sort_st> sort_pix(cv::Mat image)
{
    cout << "排序" << endl;
    int w = image.cols; // - 宽
    int h = image.rows; // - 高
    vector<sort_st> sort_array(w * h);
    for (int i = 0; i < sort_array.size(); ++i)
    {
        sort_array[i].index = i;
    }

    for (int i = 0; i < w * h; ++i)
    {

        int channels = image.channels();

        if (channels == 1)
        {

            //得到初始位置的迭代器
            cv::Mat_<uchar>::iterator it = image.begin<uchar>();
            //得到终止位置的迭代器
            cv::Mat_<uchar>::iterator itend = image.end<uchar>();

            // - 获得像素
            int pixel = *(it + i);
            sort_array[i].pixel = pixel;
        }
    }
    sort(sort_array.begin(), sort_array.end(), compare);
    //   - 输出看一下是否是排序
    /*
        for (int i = 0; i < w * h; ++i)
        {
            cout << sort_array[i].index << ":" << sort_array[i].pixel << " **";
        }
     */
    return sort_array;
}

// step 3 灰度级别分段
int split_pix(vector<sort_st> sort_pix_index)
{
    cout << "灰度级别分段" << endl;
    int x = 0;
    x = int(sort_pix_index[sort_pix_index.size()].pixel - sort_pix_index[0].pixel) + 1;
    cout << "灰度等级:" << x << endl;
    return x;
}

int split_pix(cv::Mat &image0)
{
    int x = 0;
    double minVal = 0.0;
    double maxVal = 0.0;
    cv::minMaxLoc(image0, &minVal, &maxVal);
    x = int((maxVal - minVal)) + 1;
    cout << "原始灰度等级:" << minVal << ":" << maxVal << "共" << x << "个级别" << endl;
    return x;
}

// step 4 灰度级映射
cv::Mat balanced_mapping(cv::Mat image, vector<sort_st> sort_pix_index, int x)
{
    cout << "灰度级映射" << endl;
    cv::Mat balanced_mapping_image = image * 0;
    int pixel = 0;
    int level = 1;
    for (int i = 0; i < sort_pix_index.size(); ++i)
    {
        //得到初始位置的迭代器
        cv::Mat_<uchar>::iterator it = balanced_mapping_image.begin<uchar>();
        int xxx = sort_pix_index[i].index;
        *(it + xxx) = pixel;
        if (i > sort_pix_index.size() * level / x)
        {
            ++pixel;
            ++level;
        }
        // cout << pixel << "  " << xxx << "  ";
    }

    return balanced_mapping_image;
}

//绘制直方图，src为输入的图像，histImage为输出的直方图，name是输出直方图的窗口名称
double drawHistImg(cv::Mat &src, cv::Mat &histImage, std::string name, double maxValue0)
{
    const int bins = 256;
    int hist_size[] = {bins};
    float range[] = {0, 256};
    const float *ranges[] = {range};
    cv::MatND hist;
    int channels[] = {0};

    cv::calcHist(&src, 1, channels, cv::Mat(), hist, 1, hist_size, ranges, true, false);

    double maxValue;
    cv::minMaxLoc(hist, 0, &maxValue, 0, 0);
    int scale = 1;
    int histHeight = 256 * 3;

    if (maxValue0 != 0)
    {
        maxValue = maxValue0;
    }

    for (int i = 0; i < 2 * bins; ++i)
    {
        float binValue = hist.at<float>(i);
        int height = cvRound(binValue * histHeight / maxValue);
        cv::rectangle(histImage, cv::Point(5 * i * scale, histHeight), cv::Point((5 * i + 1) * scale, histHeight - height), cv::Scalar(255));

        cv::imshow(name, histImage);
    }
    return maxValue;
}

int main(int argc, char **argv)
{
    double k = atof(argv[1]);//系数
    string image_name = string(argv[2]) + ".png";//图片名称（png格式）
    cout << image_name;
    cv::Mat image0;
    image0 = cv::imread(image_name, 0);
    cv::Mat gray;
    // cvtColor(image0, gray, CV_BGR2GRAY);
    // cv::imshow(image_name, gray);
    // cv::imwrite(image_name, gray);

    cv::imshow("原图像", image0);
    cv::waitKey(1000);

    // step 1 计算邻域测度
    cout << "step 1" << endl;
    cv::Mat Neighborhood_measurement_space;
    Neighborhood_measurement_space = Neighborhood_measurement(k, image0);
    image_name = string(argv[2]) + "Neighborhood_measurement_space.png";
    cv::imwrite(image_name, Neighborhood_measurement_space);

    // step 2 获得像素值排序的索引
    cout << "step 2 " << endl;
    vector<sort_st> sort_pix_index;
    sort_pix_index = sort_pix(Neighborhood_measurement_space);

    // step 3 获取像素灰度级
    cout << "step 3 " << endl;
    int x = 0;
    x = split_pix(image0);

    // step 4 灰度级映射R
    cout << "step 4 " << endl;
    cv::Mat result;
    result = balanced_mapping(image0, sort_pix_index, 256);
    cv::imshow("结果", result);
    cv::waitKey(1000);
    image_name = string(argv[2]) + "result.png";
    cv::imwrite(image_name, result);

    // step 5 绘制灰度直方图
    double maxValue0 = 0;
    cv::Mat image0_HistImage = cv::Mat::zeros(256 * 3, 256 * 5, CV_8UC1);
    cv::Mat Neighborhood_measurement_space_HistImage = cv::Mat::zeros(256 * 3, 256 * 5, CV_8UC1);
    cv::Mat result_HistImage = cv::Mat::zeros(256 * 3, 256 * 5, CV_8UC1);
    maxValue0 = drawHistImg(image0, image0_HistImage, "原图像直方图", maxValue0);
    maxValue0 = drawHistImg(Neighborhood_measurement_space, Neighborhood_measurement_space_HistImage, "邻域测度空间图像直方图", maxValue0);
    maxValue0 = drawHistImg(result, result_HistImage, "结果图像直方图", maxValue0);
    cvWaitKey(1000);
    image_name = string(argv[2]) + "image0_HistImage.png";
    cv::imwrite(image_name, image0_HistImage);
    image_name = string(argv[2]) + "Neighborhood_measurement_space_HistImage.png";
    cv::imwrite(image_name, Neighborhood_measurement_space_HistImage);
    image_name = string(argv[2]) + "result_HistImage.png";
    cv::imwrite(image_name, result_HistImage);

    return 0;
}