#include "../stdafx.h"

#include <opencv2/opencv.hpp>

extern "C"
{
#include <Kernels/ref.h>
#include <types.h>
}

#include "../DemoEngine.h"

///////////////////////////////////////////////////////////////////////////////
//@brief Demonstration of MatchTemplate function
class demo_MatchTemplate : public IDemoCase {
public:
   ///@brief default ctor
   demo_MatchTemplate()
      : m_method(SQDIFF) {
      // nothing to do
   }

   ///@see IDemoCase::ReplyName
   virtual std::string ReplyName() const override {
      return "MatchTemplate";
   }

private:
   ///@see IDemoCase::execute
   virtual void execute() override;

   ///@brief provide interactive demo
   static void applyParameters(int pos, void* data);

private:
   match_template_method_enum m_method;
   cv::Mat m_src_image;
   cv::Mat m_tmpl_image;
};

///////////////////////////////////////////////////////////////////////////////
namespace
{
   const std::string m_openVXWindow    = "openVX";
   const std::string m_openCVWindow    = "openCV";
   const std::string m_originalWindow  = "original";
   const std::string m_diffWindow      = "Diff of " + m_openVXWindow + " and " + m_openCVWindow;
}

///////////////////////////////////////////////////////////////////////////////
void demo_MatchTemplate::execute()
{
   cv::namedWindow(m_originalWindow, CV_WINDOW_NORMAL);
   cv::namedWindow(m_openVXWindow, CV_WINDOW_NORMAL);
   cv::namedWindow(m_openCVWindow, CV_WINDOW_NORMAL);
   cv::namedWindow(m_diffWindow, CV_WINDOW_NORMAL);

   const std::string scr_img_path = "../../Image/Solvay_conference_1927.png";
   const std::string tmpl_img_path = "../../Image/apple_cutted.png";
   m_src_image = cv::imread(scr_img_path, CV_LOAD_IMAGE_GRAYSCALE);
   cv::imshow(m_originalWindow, m_src_image);
   m_tmpl_image = cv::imread(tmpl_img_path, CV_LOAD_IMAGE_GRAYSCALE);

   applyParameters(m_method, this);
   cv::waitKey(0);
}

///////////////////////////////////////////////////////////////////////////////
void demo_MatchTemplate::applyParameters(int, void* data)
{
   auto demo = static_cast<demo_MatchTemplate*>(data);

   const cv::Size img_size(demo->m_src_image.cols, demo->m_src_image.rows);
   const cv::Size tmpl_size(demo->m_tmpl_image.cols, demo->m_tmpl_image.rows);

   ///@{ OPENCV
   cv::Mat cv_image;
   cv::matchTemplate(demo->m_src_image, demo->m_tmpl_image, cv_image, (int)(demo->m_method));
   cv::imshow(m_openCVWindow, cv_image);
   ///@}

   ///@{ OPENVX
   _vx_image src_vx_image = {
      demo->m_src_image.data,
      uint32_t(img_size.width),
      uint32_t(img_size.height),
      VX_DF_IMAGE_U8,
      VX_COLOR_SPACE_DEFAULT
   };

   _vx_image tmpl_vx_image = {
      demo->m_tmpl_image.data,
      uint32_t(tmpl_size.width),
      uint32_t(tmpl_size.height),
      VX_DF_IMAGE_U8,
      VX_COLOR_SPACE_DEFAULT
   };

   uint8_t* outVXImage = static_cast<uint8_t*>(calloc(img_size.width* img_size.height, sizeof(uint8_t)));
   _vx_image dstVXImage = {
      outVXImage,
      uint32_t(img_size.width),
      uint32_t(img_size.height),
      VX_DF_IMAGE_U8,
      VX_COLOR_SPACE_DEFAULT
   };

   ref_MatchTemplate(&src_vx_image,  &tmpl_vx_image, &dstVXImage, demo->m_method);

   const cv::Mat vxImage = cv::Mat(img_size, CV_8UC1, outVXImage);
   cv::imshow(m_openVXWindow, vxImage);
   ///@}

   // Show difference of OpenVX and OpenCV
   const cv::Mat diffImage(img_size, CV_8UC1);
   cv::absdiff(vxImage, cv_image, diffImage);
   cv::imshow(m_diffWindow, diffImage);
}

///////////////////////////////////////////////////////////////////////////////
IDemoCasePtr CreateMatchTemplateDemo()
{
   return std::make_unique<demo_MatchTemplate>();
}
