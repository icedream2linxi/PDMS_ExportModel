using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	[Class(Table = "sphere")]
	public class Sphere
	{
		[Id(0, TypeType = typeof(int))]
		[Key(1)]
		[Generator(2, Class = "native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Center")]
		public virtual Point Center { get; set; }

		[Point(After = typeof(IdAttribute), Name = "BottomNormal", Prefix = "bottom_normal")]
		public virtual Point BottomNormal { get; set; }

		[Property(Column = "radius")]
		public virtual double Radius { get; set; }

		[Property(Column = "angle")]
		public virtual double Angle { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }
	}
}
