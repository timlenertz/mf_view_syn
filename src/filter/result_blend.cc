/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "result_blend.h"
#include <tuple>

namespace vs {

using namespace mf;

std::pair<real, real> result_blend_filter::weights_(job_type& job) const {
	const auto& left_cam = job.param(left_source_camera);
	const auto& right_cam = job.param(right_source_camera);
	const auto& virtual_cam = job.param(virtual_camera);

	real left_distance = (left_cam.absolute_pose().position - virtual_cam.absolute_pose().position).norm();
	real right_distance = (right_cam.absolute_pose().position - virtual_cam.absolute_pose().position).norm();
	real sum = left_distance + right_distance;
	
	return { left_distance / sum, right_distance / sum };
}


void result_blend_filter::setup() {
	MF_ASSERT(left_image_input.frame_shape() == right_image_input.frame_shape());
	shape_ = left_image_input.frame_shape();
	virtual_image_output.define_frame_shape(shape_);
}


void result_blend_filter::process(job_type& job) {	
	auto virtual_out = job.out(virtual_image_output);
	auto left_in = job.in(left_image_input);
	auto right_in = job.in(right_image_input);
	
	real left_weight, right_weight;
	std::tie(left_weight, right_weight) = weights_(job);
	bool prefer_left = (left_weight >= right_weight);
	bool prefer_right = (right_weight >= left_weight);
	
	for(auto coord : make_ndspan(shape_)) {
		masked_color_type left_color = left_in.at(coord);
		masked_color_type right_color = right_in.at(coord);
		
		masked_color_type virtual_color;

		if(left_color.is_null() && right_color.is_null())
			virtual_color = masked_color_type::null();
		else if(left_color.is_null() || (left_color.get_flag(unstable_pixel_flag) && prefer_right))
			virtual_color = right_color;
		else if(right_color.is_null() || (right_color.get_flag(unstable_pixel_flag) && prefer_left))
			virtual_color = left_color;
		else
			virtual_color = color_blend(left_color, left_weight, right_color, right_weight);
		
		virtual_out.at(coord) = virtual_color;
	}

}


}

/*


  // pixels which will be replaced by pixels synthesized from right view
   cvAnd(m_pcViewSynthesisLeft->getUnstablePixels(), m_pcViewSynthesisRight->getSynthesizedPixels(), m_imgMask[3]); 

  if(ViewBlending==1)
  {
    if(m_dWeightLeft>=m_dWeightRight) 
    { 
      cvCopy(m_pcViewSynthesisRight->getVirtualImage(), m_pcViewSynthesisLeft->getVirtualImage(), m_imgMask[3]); 
    } 
    else 
    { 
      cvCopy(m_pcViewSynthesisRight->getVirtualImage(), m_pcViewSynthesisLeft->getVirtualImage(), m_pcViewSynthesisRight->getSynthesizedPixels()); 
    } 
  } else {
    cvCopy(m_pcViewSynthesisRight->getVirtualImage(), m_pcViewSynthesisLeft->getVirtualImage(), m_imgMask[3]); 
  }

  // pixels which will be replaced by pixels synthesized from left view
  cvAnd(m_pcViewSynthesisRight->getUnstablePixels(), m_pcViewSynthesisLeft->getSynthesizedPixels(), m_imgMask[4]); 

  if(ViewBlending==1)
  {
    if(m_dWeightLeft<=m_dWeightRight) 
    { 
      cvCopy(m_pcViewSynthesisLeft->getVirtualImage(), m_pcViewSynthesisRight->getVirtualImage(), m_imgMask[4]); 
    } 
    else 
    { 
      cvCopy(m_pcViewSynthesisLeft->getVirtualImage(), m_pcViewSynthesisRight->getVirtualImage(), m_pcViewSynthesisLeft->getSynthesizedPixels()); 
    } 
  } else {
  cvCopy(m_pcViewSynthesisLeft->getVirtualImage(), m_pcViewSynthesisRight->getVirtualImage(), m_imgMask[4]); 
  }
  
  // pixels which couldn't be synthesized from both left and right -> inpainting
  cvAnd(m_pcViewSynthesisLeft->getHolePixels(), m_pcViewSynthesisRight->getHolePixels(), m_imgMask[2]);
#ifdef POZNAN_DEPTH_BLEND
#define GETUCHAR(x,ptr) (((unsigned char*)x)[ptr])
  IplImage *DepthLeft = m_pcViewSynthesisLeft ->getVirtualDepthMap();
  IplImage *DepthRight = m_pcViewSynthesisRight ->getVirtualDepthMap();
  IplImage *ImageLeft = m_pcViewSynthesisLeft ->getVirtualImage();
  IplImage *ImageRight = m_pcViewSynthesisRight ->getVirtualImage();
  IplImage *SynthesizedLeft = m_pcViewSynthesisLeft ->getHolePixels();
  IplImage *SynthesizedRight = m_pcViewSynthesisRight ->getHolePixels();
  for(int h=0; h<m_uiHeight; h++)
  {
    for(int w=0; w<m_uiWidth; w++)
    {
      int ptv = w + h * m_uiWidth;
      m_imgBlended->imageData[ptv*3+0] = 0;
      m_imgBlended->imageData[ptv*3+1] = 0;
      m_imgBlended->imageData[ptv*3+2] = 0;
      if(m_imgMask[2]->imageData[ptv]!=0) continue;
      if((abs(DepthLeft->imageData[ptv]-DepthRight->imageData[ptv])< m_iDepthBlendDiff )) //5
      {
        ((unsigned char*)m_imgBlended->imageData)[ptv*3+0] = CLIP3((((unsigned char*)ImageLeft->imageData)[ptv*3+0]*m_dWeightLeft+((unsigned char*)ImageRight->imageData)[ptv*3+0]*m_dWeightRight)/(m_dWeightLeft+m_dWeightRight),0,255);  
        ((unsigned char*)m_imgBlended->imageData)[ptv*3+1] = CLIP3((((unsigned char*)ImageLeft->imageData)[ptv*3+1]*m_dWeightLeft+((unsigned char*)ImageRight->imageData)[ptv*3+1]*m_dWeightRight)/(m_dWeightLeft+m_dWeightRight),0,255);
        ((unsigned char*)m_imgBlended->imageData)[ptv*3+2] = CLIP3((((unsigned char*)ImageLeft->imageData)[ptv*3+2]*m_dWeightLeft+((unsigned char*)ImageRight->imageData)[ptv*3+2]*m_dWeightRight)/(m_dWeightLeft+m_dWeightRight),0,255);
      }
      else if((DepthLeft->imageData[ptv]<DepthRight->imageData[ptv])) //Fix to compare z
      {
        m_imgBlended->imageData[ptv*3+0] = ImageLeft->imageData[ptv*3+0];
        m_imgBlended->imageData[ptv*3+1] = ImageLeft->imageData[ptv*3+1];
        m_imgBlended->imageData[ptv*3+2] = ImageLeft->imageData[ptv*3+2];
      }
      else /*if((m_imgMask[3]->imageData[ptv]!=0))* /
      {
        m_imgBlended->imageData[ptv*3+0] = ImageRight->imageData[ptv*3+0];
        m_imgBlended->imageData[ptv*3+1] = ImageRight->imageData[ptv*3+1];
        m_imgBlended->imageData[ptv*3+2] = ImageRight->imageData[ptv*3+2];
      }
      
    }
  }
  //m_imgBlended
#else
  cvAddWeighted(m_pcViewSynthesisLeft->getVirtualImage(), m_dWeightLeft, m_pcViewSynthesisRight->getVirtualImage(), m_dWeightRight, 0, m_imgBlended);
#endif
  //cvSaveImage("Mask2.bmp",m_imgMask[2]);
  cvSet(m_imgBlended, CV_RGB(0, 128, 128), m_imgMask[2]); 
  cvInpaint(m_imgBlended, m_imgMask[2], m_imgInterpolatedView, 5, CV_INPAINT_NS); 
  //inpaint(m_imgBlended, m_imgMask[2], m_imgInterpolatedView, 5, INPAINT_NS); 

*/
