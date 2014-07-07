using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	[Class(Table = "pyramid")]
	public class Pyramid
	{
		[Id(0, TypeType = typeof(int))]
		[Key(1)]
		[Generator(2, Class = "native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Org")]
		public virtual Point Org { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Height")]
		public virtual Point Height { get; set; }

		[Point(After = typeof(IdAttribute), Name = "XAxis")]
		public virtual Point XAxis { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Offset")]
		public virtual Point Offset { get; set; }

		[Property(Column = "bottom_xlen")]
		public virtual double BottomXLen { get; set; }

		[Property(Column = "bottom_ylen")]
		public virtual double BottomYLen { get; set; }

		[Property(Column = "top_xlen")]
		public virtual double TopXLen { get; set; }

		[Property(Column = "top_ylen")]
		public virtual double TopYLen { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }

	}
}
