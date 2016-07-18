/*
 * Copyright (C) 2016, Australian Centre for Robotic Vision, ACRV
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Osnabrück University nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *  Created on: 11.07.2016
 *
 *      Authors:
 *         Trung T. Pham <trung.pham@adelaide.edu.au>
 *         Markus Eich <markus.eich@qut.edu.au>
 *
 *
 */

#include <iostream>

#include <segmentation/segmentation.hpp>



using namespace std;

using namespace APC;


int main(int argc, char** argv){


    Segmentation seg;
    Config config;

    ///  Parse Arguments needed for computation
    pcl::console::parse (argc, argv, "-v", config.voxel_resolution);
    pcl::console::parse (argc, argv, "-s", config.seed_resolution);
    pcl::console::parse (argc, argv, "-c", config.color_importance);
    pcl::console::parse (argc, argv, "-z", config.spatial_importance);
    pcl::console::parse (argc, argv, "-n", config.normal_importance);

    pcl::console::parse (argc, argv, "-lc", config.label_cost);
    pcl::console::parse (argc, argv, "-sc", config.smooth_cost);
    pcl::console::parse (argc, argv, "-oc", config.outlier_cost);

    if (pcl::console::find_switch (argc, argv, "-h")){
      PCL_INFO ("Options: \n \t -v : voxel resolution\n \t -s : seed resolution\n \t -c : color importance\n \t -z : spatial importance \n \t -n : normal importance\n \t -lc : label cost\n \t -sc : smooth cost\n \t -oc : outlier cost\n");	       
	      exit (0);
    }
	              

    seg.setConfig(config);


    /// -----------------------------------|  Preparations  |-----------------------------------

    if (argc<2){
        PCL_ERROR ("ERROR: wrong number or arguments provided.Use -h to show options.\n");
        exit (-1);
    }

    PCL_INFO ("Loading pointcloud\n");
    pcl::PointCloud<PointT>::Ptr input_cloud_ptr (new pcl::PointCloud<PointT>);



    /// Get pcd path from command line
    std::string pcd_filename = argv[1];
    if (pcl::io::loadPCDFile (pcd_filename, *input_cloud_ptr))
    {
        PCL_ERROR ("ERROR: Could not read input point cloud %s.\n", pcd_filename.c_str ());
        return (3);
    }

    std::vector< int > index;
    if (!input_cloud_ptr->is_dense){
        PCL_WARN("Point data not dense, eating NaNs\n");
        pcl::removeNaNFromPointCloud<PointT>(*input_cloud_ptr,*input_cloud_ptr,index);
        PCL_INFO ("Done making cloud\n");
    }
    std::cerr << "Number of points: " << input_cloud_ptr->size() << std::endl;


    seg.setPointCloud(input_cloud_ptr);

    seg.doSegmentation();

        pcl::PointCloud<pcl::PointXYZL>::Ptr segmented_cloud_ptr;

        segmented_cloud_ptr=seg.getSegmentedPointCloud();

        bool output_specified = pcl::console::find_switch (argc, argv, "-o");
        if (output_specified)
        {
            // Check output already exists. Create outputname if not given
            std::string outputname ("");
            pcl::console::parse (argc, argv, "-o", outputname);
            // If no filename is given, get output filename from inputname (strip seperators and file extension)
            if (outputname.empty () || (outputname.at (0) == '-'))
            {
                outputname = pcd_filename;
                size_t dot = outputname.find_last_of ('.');
                if (dot != std::string::npos)
                    outputname = outputname.substr (0, dot);
            }

            outputname+="_seg.pcd";
            PCL_INFO ("Saving output\n");
            bool save_binary_pcd = false;
            pcl::io::savePCDFile (outputname, *segmented_cloud_ptr, save_binary_pcd);
        }

        /// -----------------------------------|  Visualization  |-----------------------------------
        bool show_visualization = (not pcl::console::find_switch (argc, argv, "-novis"));
        if (show_visualization)
        {
            /// Configure Visualizer
            pcl::visualization::PCLVisualizer::Ptr viewer (new pcl::visualization::PCLVisualizer ("3D Viewer"));
            viewer->setBackgroundColor (0, 0, 0);
            viewer->addPointCloud (segmented_cloud_ptr, "Segmented point cloud");

            PCL_INFO ("Loading viewer\n");
            while (!viewer->wasStopped ())
            {
                viewer->spinOnce (100);
                viewer->updatePointCloud (segmented_cloud_ptr, "Segmented point cloud");
                boost::this_thread::sleep (boost::posix_time::microseconds (100000));
            }
        }
    return 0;
}
