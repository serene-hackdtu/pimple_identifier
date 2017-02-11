#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_processing.h>
#include <dlib/data_io.h>
#include <iostream>
#include <fstream>
using namespace std;
using namespace dlib;
int main(int argc, char** argv)
{  

    try
    {
        if (argc != 2)
        {
            cout << "Give the path to the examples/faces directory as the argument to this" << endl;
            cout << "program.  For example, if you are in the examples folder then execute " << endl;
            cout << "this program by running: " << endl;
            cout << "   ./object_detector" << endl;
            cout << endl;
            return 0;
        }
        const std::string faces_directory = argv[1];
        dlib::array<array2d<unsigned char> > images_train, images_test;
        std::vector<std::vector<rectangle> > face_boxes_train, face_boxes_test;
        load_image_dataset(images_train, face_boxes_train, faces_directory+"/training.xml");
        upsample_image_dataset<pyramid_down<2> >(images_train, face_boxes_train);
        add_image_left_right_flips(images_train, face_boxes_train);
        cout << "num training images: " << images_train.size() << endl;
        typedef scan_fhog_pyramid<pyramid_down<6> > image_scanner_type; 
        image_scanner_type scanner;
        scanner.set_detection_window_size(80, 80); 
        structural_object_detection_trainer<image_scanner_type> trainer(scanner);
        trainer.set_num_threads(2);
        trainer.set_c(1); 
        trainer.be_verbose();
        trainer.set_epsilon(0.0001);
        object_detector<image_scanner_type> detector = trainer.train(images_train, face_boxes_train);
        cout << "training results: " << test_object_detection_function(detector, images_train, face_boxes_train) << endl;
        image_window hogwin(draw_fhog(detector), "Learned fHOG detector");
        image_window win; 
        for (unsigned long i = 0; i < images_test.size(); ++i)
        {
            // Run the detector and get the face detections.
            std::vector<rectangle> dets = detector(images_test[i]);
            win.clear_overlay();
            win.set_image(images_test[i]);
            win.add_overlay(dets, rgb_pixel(255,0,0));
            cout << "Hit enter to process the next image..." << endl;
            cin.get();
        }
        serialize("face_detector.svm") << detector;
        object_detector<image_scanner_type> detector2;
        deserialize("face_detector.svm") >> detector2;
        std::vector<object_detector<image_scanner_type> > my_detectors;
        my_detectors.push_back(detector);
        std::vector<rectangle> dets = evaluate_detectors(my_detectors, images_train[0]); 
        cout << "num filters: "<< num_separable_filters(detector) << endl;
        detector = threshold_filter_singular_values(detector,0.1); 

    }
    catch (exception& e)
    {
        cout << "\nexception thrown!" << endl;
        cout << e.what() << endl;
    }
}