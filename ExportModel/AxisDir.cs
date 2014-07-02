using Aveva.Pdms.Geometry;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ExportModel
{
	public class AxisDir
	{
		public Position Pos { get; set; }
		public Direction Dir { get; set; }

		public AxisDir()
		{

		}

		public AxisDir(Position pos, Direction dir)
		{
			Pos = pos;
			Dir = dir;
		}
	}
}
