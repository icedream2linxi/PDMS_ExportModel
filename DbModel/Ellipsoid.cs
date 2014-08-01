using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	[Class(Table = "ellipsoid")]
	public class Ellipsoid
	{
		[Id(0, TypeType = typeof(int))]
		[Key(1)]
		[Generator(2, Class = "native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Center")]
		public virtual Point Center { get; set; }

		[Point(After = typeof(IdAttribute), Name = "ALen", Prefix = "a_len")]
		public virtual Point ALen { get; set; }

		[Property(Column = "b_radius")]
		public virtual double BRadius { get; set; }

		[Property(Column = "angle")]
		public virtual double Angle { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }
	}
}
