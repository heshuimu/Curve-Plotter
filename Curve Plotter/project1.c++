// project1.c++

#include "GLFWController.h"
#include "ModelView.h"
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[])
{
	GLFWController c(argv[0]);
	c.reportVersions(std::cout);

	ShaderIF* sIF = new ShaderIF("shaders/project1.vsh", "shaders/project1.fsh");
	
	float xmin = std::numeric_limits<float>::max();
	float xmax = std::numeric_limits<float>::min();
	float ymin = std::numeric_limits<float>::max();
	float ymax = std::numeric_limits<float>::min();
	
	std::string axis_x, axis_y;
	
	if(argc > 1)
	{
		std::ifstream data(argv[1]);
		if(data.is_open())
		{
			std::string size_str;
			getline(data, size_str);
			int dataSize = std::stoi(size_str);
			std::cout << dataSize <<"\n";
			
			getline(data, axis_x);
			getline(data, axis_y);
			std::cout << axis_x << ", " << axis_y <<"\n";
			
			float* data_x = new float[dataSize];
			std::string line;
			getline(data, line);
			std::stringstream ss(line);
			for (int i = 0; i < dataSize; i++)
			{
				ss >> data_x[i];
				if(data_x[i] > xmax)
					xmax = data_x[i];
				if(data_x[i] < xmin)
					xmin = data_x[i];
			}
			
			while(getline(data, line))
			{
				vec2_f* vecs = new vec2_f[dataSize];
				std::stringstream ss1(line);
				for (int i = 0; i < dataSize; i++)
				{
					ss1 >> vecs[i][1];
					vecs[i][0] = data_x[i];
					printf("(%f, %f)\n", vecs[i][0], vecs[i][1]);
					if(vecs[i][1] > ymax)
						ymax = vecs[i][1];
					if(vecs[i][1] < ymin)
						ymin = vecs[i][1];
				}
				
				c.addModel(new ModelView(sIF, vecs, dataSize, false));
				delete[] vecs;
			}
			
			printf("Overall Limits: %f, %f, %f, %f\n", xmin, xmax, ymin, ymax);

			
			//y axis
			if(xmin <= 0 && xmax >= 0)
			{
				vec2_f y_axis_vecs[] = {{0, ymin},{0, ymax}};
				c.addModel(new ModelView(sIF, y_axis_vecs, 2, true));
			}
			else
			{
				float y_axis_x = (xmin+xmax)/2;
				vec2_f y_axis_vecs[] = {{y_axis_x, ymin},{y_axis_x, ymax}};
				c.addModel(new ModelView(sIF, y_axis_vecs, 2, true));
			}
			
			//x axis
			if(ymin <= 0 && ymax >= 0)
			{
				vec2_f x_axis_vecs[] = {{xmin, 0},{xmax, 0}};
				c.addModel(new ModelView(sIF, x_axis_vecs, 2, true));
			}
			else
			{
				float x_axis_y = (ymin+ymax)/2;
				vec2_f x_axis_vecs[] = {{xmin, x_axis_y},{xmax, x_axis_y}};
				c.addModel(new ModelView(sIF, x_axis_vecs, 2, true));
			}
		}
		else
		{
			printf("-----------------------\nInput file could not be opened\n-----------------------\n");
			return 2;
		}
	}
	else
	{
		printf("-----------------------\nAn Input file is needed\n-----------------------\n");
		return 1;
	}

	// initialize 2D viewing information:
	// Get the overall scene bounding box in Model Coordinates:
	double xyz[6]; // xyz limits, even though this is 2D
	c.getOverallMCBoundingBox(xyz);
	// Simplest case: Just tell the ModelView we want to see it all:
	ModelView::setMCRegionOfInterest(xyz);
	
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	c.run();

	delete sIF;

	return 0;
}
