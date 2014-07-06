using Aveva.Pdms.Geometry;
using Aveva.Pdms.Geometry.Implementation;
using Aveva.Pdms.Maths.Geometry;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ExportModel
{
	class GeometryUtility
	{
		public static D3Point Org = D3Point.Create();
		public static D3Transform ToD3Transform(Orientation ori, Position pos)
		{
			return D3Transform.Create(ToD3MatrixRef(ori), D3Vector.Create(pos.X, pos.Y, pos.Z));
		}

		public static D3Matrix ToD3MatrixRef(Orientation ori)
		{
			return ((OrientationImpl)ori).CoreValue();
		}

		public static D3Vector ToD3VectorRef(Direction dir)
		{
			return ((DirectionImpl)dir).CoreValue();
		}

		public static D3Vector ToD3Vector(Position pos)
		{
			return D3Vector.Create(pos.X, pos.Y, pos.Z);
		}

		public static D3Point ToD3Point(Position pos)
		{
			return D3Point.Create(pos.X, pos.Y, pos.Z);
		}
	}
}
