#include <math.h>
#include "Lanczos.h"
#include "stlutil.h"
#define Pi 3.14159

//#define __DBG__
#include "Debug.h"

#ifdef __DBG__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tchar.h>
#endif

namespace CGrLanczos
{
	// キャストすると遅いのでbyte -> double 変換マップを作成
	static double byte2d[] = {
		+ 0.0,  1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,  9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
		+16.0, 17.0, 18.0, 19.0, 20.0, 21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0, 29.0, 30.0, 31.0,
		+32.0, 33.0, 34.0, 35.0, 36.0, 37.0, 38.0, 39.0, 40.0, 41.0, 42.0, 43.0, 44.0, 45.0, 46.0, 47.0,
		+48.0, 49.0, 50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0, 61.0, 62.0, 63.0,
		+64.0, 65.0, 66.0, 67.0, 68.0, 69.0, 70.0, 71.0, 72.0, 73.0, 74.0, 75.0, 76.0, 77.0, 78.0, 79.0,
		+80.0, 81.0, 82.0, 83.0, 84.0, 85.0, 86.0, 87.0, 88.0, 89.0, 90.0, 91.0, 92.0, 93.0, 94.0, 95.0,
		+96.0, 97.0, 98.0, 99.0,100.0,101.0,102.0,103.0,104.0,105.0,106.0,107.0,108.0,109.0,110.0,111.0,
		112.0,113.0,114.0,115.0,116.0,117.0,118.0,119.0,120.0,121.0,122.0,123.0,124.0,125.0,126.0,127.0,
		128.0,129.0,130.0,131.0,132.0,133.0,134.0,135.0,136.0,137.0,138.0,139.0,140.0,141.0,142.0,143.0,
		144.0,145.0,146.0,147.0,148.0,149.0,150.0,151.0,152.0,153.0,154.0,155.0,156.0,157.0,158.0,159.0,
		160.0,161.0,162.0,163.0,164.0,165.0,166.0,167.0,168.0,169.0,170.0,171.0,172.0,173.0,174.0,175.0,
		176.0,177.0,178.0,179.0,180.0,181.0,182.0,183.0,184.0,185.0,186.0,187.0,188.0,189.0,190.0,191.0,
		192.0,193.0,194.0,195.0,196.0,197.0,198.0,199.0,200.0,201.0,202.0,203.0,204.0,205.0,206.0,207.0,
		208.0,209.0,210.0,211.0,212.0,213.0,214.0,215.0,216.0,217.0,218.0,219.0,220.0,221.0,222.0,223.0,
		224.0,225.0,226.0,227.0,228.0,229.0,230.0,231.0,232.0,233.0,234.0,235.0,236.0,237.0,238.0,239.0,
		240.0,241.0,242.0,243.0,244.0,245.0,246.0,247.0,248.0,249.0,250.0,251.0,252.0,253.0,254.0,255.0,
	};
	struct Weight {
		int idx = 0;
		double weight = 0.0;
	};
	using WeightList = std::simple_array<Weight>;
	using WeightMap = std::simple_array<WeightList>;
	/*
	  Lanczos(x) = sinc(x) * sinc(x/n) , |x| <= n
	             = 0 , |x| > n
	  sinc(x) = sin(PI * x) / (PI * x)
	 */
	static double sinc(double x)
	{
		return sin(Pi*x)/(Pi*x);
	}
	static double lanczos(double x, double n)
	{
		if(x == 0.0){
			return 1.0;
		} else if(fabs(x) > n){
			return 0.0;
		} else {
			return sinc(x) * sinc(x/n);
		}
	}
	static int filter(double x, double total)
	{
		if(total != 0.0){
			x /= total;
		}
		if(x < 0.0){
			return 0;
		} else if(x > 255.0){
			return 255;
		}
		return static_cast<int>(x);
	}
	static void filter(BYTE &b, double x)
	{
		if(x < 1.0){ // int へのキャストで 1.0未満は 切り捨てられるので
			b = 0;
		} else if(x > 255.0){
			b = 255;
		} else {
			b = static_cast<int>(x);
		}
	}
	static void makeUpLanczosMap(WeightMap &lanczos_map, int n, double scale, int src_length)
	{
		int dist_length  = static_cast<int>(static_cast<double>(src_length) * scale);
		if(dist_length <= 0){ return; }
		lanczos_map.resize(dist_length);
		auto *pwl = &lanczos_map[0];
		int idx = 0;
		for(; idx<dist_length; ++idx, ++pwl){
			auto &&wl = *pwl;
			auto l = (static_cast<double>(idx) + 0.5) / scale;
			int s = max(static_cast<int>(l - static_cast<double>(n)), 0         );
			int e = min(static_cast<int>(l + static_cast<double>(n)), src_length);
			wl.resize(e-s+1);
			auto *pw = &wl[0];
			for(; s<e; ++s, ++pw){
				*pw = Weight{s, lanczos(fabs((static_cast<double>(s) + 0.5) - l), n)};
			}
		}
	}
	static void makeDownLanczosMap(WeightMap &lanczos_map, double dn, double scale, int src_length)
	{
		int dist_length  = static_cast<int>(static_cast<double>(src_length) * scale);
		if(dist_length <= 0){ return; }
		lanczos_map.resize(dist_length);
		auto *pwl = &lanczos_map[0];
		double l = 0.5;
		for(; dist_length>0; ++l, ++pwl, --dist_length){
			auto &&wl = *pwl;
			int s = static_cast<int>(max((l - dn) / scale, 0         ));
			int e = static_cast<int>(min((l + dn) / scale, src_length));
			auto ds = (static_cast<double>(s) + 0.5) * scale - l;
			wl.resize(e-s+1);
			auto *pw = &wl[0];
			for(; s<e; ++s, ds+=scale, ++pw){
				*pw = Weight{s, lanczos(fabs(ds), dn)};
			}
		}
	}

	void StretchBlt(CGrBitmap &dst_bmp, CGrBitmap &src_bmp, int n, double scale)
	{
		StretchBlt(dst_bmp, src_bmp, n, scale, scale);
	}
	void ForEach(CGrBitmap &src_bmp, LPRGBQUAD lp_next_dst, int dst_width, WeightMap &lanczos_map_x, WeightList *it_y, int map_y)
	{
		auto lp_org_src = src_bmp.GetBits();
		int src_width = src_bmp.Width();
		//
		int map_x_len = lanczos_map_x.size();
		auto *itb_x = &lanczos_map_x[0];
		//
		for(; map_y>0; --map_y, ++it_y, lp_next_dst += dst_width){
			auto &&lanczos_y = *it_y;
			auto lp_dst = lp_next_dst;
			//
			int lan_y_len = lanczos_y.size();
			auto *itb_ly = &lanczos_y[0];
			//
			auto *it_x = itb_x;
			for(int map_x=map_x_len; map_x>0; --map_x, ++it_x, ++lp_dst){
				auto &&lanczos_x = *it_x;
				auto &&dstRgb = *lp_dst;

				double weight_total= 0.0;
				double red         = 0.0;
				double green       = 0.0;
				double blue        = 0.0;
				double alpha       = 0.0;
				//
				int lan_x_len = lanczos_x.size();
				auto *itb_lx = &lanczos_x[0];
				//
				auto *it_ly = itb_ly;
				for(int lan_y=lan_y_len; lan_y>0; --lan_y, ++it_ly){
					auto weight_y = it_ly->weight;
					//
					auto lp_src = lp_org_src + it_ly->idx * src_width;
					auto *it_lx = itb_lx;
					for(int lan_x=lan_x_len; lan_x>0; --lan_x, ++it_lx){
						auto weight = it_lx->weight * weight_y;
						//
						auto &&srcRgb = *(lp_src+it_lx->idx);
						red   += byte2d[srcRgb.rgbRed     ] * weight;
						green += byte2d[srcRgb.rgbGreen   ] * weight;
						blue  += byte2d[srcRgb.rgbBlue    ] * weight;
						alpha += byte2d[srcRgb.rgbReserved] * weight;
						weight_total += weight;
					}
				}
				//
				if(weight_total == 0.0){
					dstRgb.rgbRed      = 0;
					dstRgb.rgbGreen    = 0;
					dstRgb.rgbBlue     = 0;
					dstRgb.rgbReserved = 0;
				} else {
					filter(dstRgb.rgbRed     , red   / weight_total);
					filter(dstRgb.rgbGreen   , green / weight_total);
					filter(dstRgb.rgbBlue    , blue  / weight_total);
					filter(dstRgb.rgbReserved, alpha / weight_total);
				}
			}
		}
	}
#include <windows.h>
#include <process.h>
	struct StretchSplit {
		LPRGBQUAD lpdst;
		int width;
		WeightList *it_y;
		int size;
		WeightMap *planczos_map_x;
		CGrBitmap *psrc_bmp;
	};
	static unsigned int __stdcall stretch_blt(void *arg)
	{
		auto *pss = static_cast<StretchSplit *>(arg);
		ForEach(*pss->psrc_bmp, pss->lpdst, pss->width, *pss->planczos_map_x, pss->it_y, pss->size);
		return 0;
	}
	void StretchBlt(CGrBitmap &dst_bmp, CGrBitmap &src_bmp, int n, double scale_w, double scale_h)
	{
		int src_width  = src_bmp.Width ();
		int src_height = src_bmp.Height();
		WeightMap lanczos_map_x;
		WeightMap lanczos_map_y;
		if(scale_w >= 1.0){
			makeUpLanczosMap  (lanczos_map_x, n, scale_w, src_width);
		} else {
			makeDownLanczosMap(lanczos_map_x, n, scale_w, src_width);
		}
		if(scale_h >= 1.0){
			makeUpLanczosMap  (lanczos_map_y, n, scale_h, src_height);
		} else {
			makeDownLanczosMap(lanczos_map_y, n, scale_h, src_height);
		}
		dst_bmp.Create(lanczos_map_x.size(), lanczos_map_y.size());
		if(lanczos_map_x.size() <= 0 || lanczos_map_y.size() <= 0){
			return;
		}
		int size = lanczos_map_y.size();
#define SPLIT_NUM 8
		static int l_split_num = SPLIT_NUM;
		StretchSplit stretch_split[SPLIT_NUM];
		auto lpdst = dst_bmp.GetBits();
		auto *it_y = &lanczos_map_y[0];
		int len = size / l_split_num;
		int next_width = dst_bmp.Width() * len;
		for(int i=0; i<l_split_num-1; ++i){
			stretch_split[i].lpdst         = lpdst          ;
			stretch_split[i].width         = dst_bmp.Width();
			stretch_split[i].it_y          = it_y           ;
			stretch_split[i].size          = len            ;
			stretch_split[i].planczos_map_x= &lanczos_map_x ;
			stretch_split[i].psrc_bmp      = &src_bmp       ;
			lpdst += next_width;
			it_y += len;
			size -= len;
		}
		auto &&last_stretch_split = stretch_split[l_split_num-1];
		last_stretch_split.lpdst         = lpdst          ;
		last_stretch_split.width         = dst_bmp.Width();
		last_stretch_split.it_y          = it_y           ;
		last_stretch_split.size          = size           ;
		last_stretch_split.planczos_map_x= &lanczos_map_x ;
		last_stretch_split.psrc_bmp      = &src_bmp       ;
		HANDLE hHandle[SPLIT_NUM] = {};
		for(int i=0; i<l_split_num; ++i){
			hHandle[i] = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, stretch_blt, static_cast<void*>(&stretch_split[i]), 0, nullptr));
		}
		for (const auto handle : hHandle) {
			::WaitForSingleObject(handle, INFINITE);
			::CloseHandle(handle);
		}
	}
};
