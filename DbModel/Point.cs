using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	public class Point
	{
		public Point()
		{

		}

		public Point(double x, double y, double z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public double X { get; set; }
		public double Y { get; set; }
		public double Z { get; set; }
	}
}
