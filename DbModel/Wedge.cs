using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;

namespace DbModel
{
	[Class(Table="wedge")]
	public class Wedge
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name="Org")]
		public virtual Point Org { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Edge1")]
		public virtual Point Edge1 { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Edge2")]
		public virtual Point Edge2 { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Height")]
		public virtual Point Height { get; set; }

		[Property(Column = "color")]
		public virtual int Color { get; set; }
	}
}
