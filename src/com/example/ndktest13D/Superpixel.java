package com.example.ndktest13D;

import java.util.ArrayList;
import java.util.List;

public class Superpixel {
	int id;
	int numNeighbour;
	Point tl;
	Point br;
	List<Point> allPixels;
	Point center;
	public Superpixel(){
		tl = new Point();
		br = new Point();
		allPixels = new ArrayList<Point>();
	}
}
