using Aveva.Pdms.Geometry;
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

		public Point(Direction dir)
		{
			X = dir.East;
			Y = dir.North;
			Z = dir.Up;
		}

		public Point(Position pos)
		{
			X = pos.X;
			Y = pos.Y;
			Z = pos.Z;
		}

		public double X { get; set; }
		public double Y { get; set; }
		public double Z { get; set; }

		public Point Mul(double s)
		{
			X *= s;
			Y *= s;
			Z *= s;
			return this;
		}

		public Point MoveBy(Direction dir, double val)
		{
			X += dir.East * val;
			Y += dir.North * val;
			Z += dir.Up * val;
			return this;
		}

		public Point MoveBy(Point dir, double val)
		{
			X += dir.X * val;
			Y += dir.Y * val;
			Z += dir.Z * val;
			return this;
		}

		public Point MoveBy(Position pos)
		{
			X += pos.X;
			Y += pos.Y;
			Z += pos.Z;
			return this;
		}
	}
}
