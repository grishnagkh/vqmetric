
#include <benchmark/benchmark.h>

#include <PSNR.hpp>
#include <SSIM.hpp>
#include <VQM.hpp>
#include <VideoCaptureReader.hpp>

static void BM_PSNR(benchmark::State& state) {
        VideoReader * rR = new VideoCaptureReader("../bench/ElephantsDream_r.mp4");
        VideoReader * pR = new VideoCaptureReader("../bench/ElephantsDream_p.mp4");
        PSNR psnr("test_psnr.log", 0);
        
        int framesPerSlice = 20;
        cv::Mat read1[framesPerSlice];
        cv::Mat read2[framesPerSlice];

        cv::Mat ref[framesPerSlice][3];
        cv::Mat proc[framesPerSlice][3];

        bool frame_avail = true;
        int i; //frames grabbed

        int nSlices = 0;
        int nFrames = 0;

        i = 0;
        while(i < framesPerSlice){
            frame_avail = rR->nextFrame(read1[i]);
            frame_avail = frame_avail && pR->nextFrame(read2[i]);
            if(!frame_avail) 
                break;
            nFrames++;
            cv::split( read1[i], ref[i] ); 
            cv::split( read2[i], proc[i] ); 
            i++;
        }	

    for (auto _ : state){
        psnr.compute(ref, proc, i);
    }
}

static void BM_SSIM(benchmark::State& state) {
        VideoReader * rR = new VideoCaptureReader("../bench/ElephantsDream_r.mp4");
        VideoReader * pR = new VideoCaptureReader("../bench/ElephantsDream_p.mp4");
        SSIM ssim("test_ssim.log", 0);
        
        int framesPerSlice = 20;
        cv::Mat read1[framesPerSlice];
        cv::Mat read2[framesPerSlice];

        cv::Mat ref[framesPerSlice][3];
        cv::Mat proc[framesPerSlice][3];

        bool frame_avail = true;
        int i; //frames grabbed

        int nSlices = 0;
        int nFrames = 0;

        i = 0;
        while(i < framesPerSlice){
            frame_avail = rR->nextFrame(read1[i]);
            frame_avail = frame_avail && pR->nextFrame(read2[i]);
            if(!frame_avail) 
                break;
            nFrames++;
            cv::split( read1[i], ref[i] ); 
            cv::split( read2[i], proc[i] ); 
            i++;
        }	

    for (auto _ : state){
        ssim.compute(ref, proc, i);
    }
}

static void BM_VQM(benchmark::State& state) {
        VideoReader * rR = new VideoCaptureReader("../bench/ElephantsDream_r.mp4");
        VideoReader * pR = new VideoCaptureReader("../bench/ElephantsDream_p.mp4");
        VQM vqm("test_vqm.log", 0, 0);
        
        int framesPerSlice = 20;
        cv::Mat read1[framesPerSlice];
        cv::Mat read2[framesPerSlice];

        cv::Mat ref[framesPerSlice][3];
        cv::Mat proc[framesPerSlice][3];

        bool frame_avail = true;
        int i; //frames grabbed

        int nSlices = 0;
        int nFrames = 0;

        i = 0;
        while(i < framesPerSlice){
            frame_avail = rR->nextFrame(read1[i]);
            frame_avail = frame_avail && pR->nextFrame(read2[i]);
            if(!frame_avail) 
                break;
            nFrames++;
            cv::split( read1[i], ref[i] ); 
            cv::split( read2[i], proc[i] ); 
            i++;
        }	

    for (auto _ : state){
        vqm.compute(ref, proc, i);
    }
}

// Register the function as a benchmark
BENCHMARK(BM_PSNR);
BENCHMARK(BM_SSIM);
BENCHMARK(BM_VQM);

BENCHMARK_MAIN();
