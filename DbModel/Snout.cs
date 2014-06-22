using NHibernate.Mapping.Attributes;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DbModel
{
	[Class(Table = "Snout")]
	public class Snout
	{
		[Id(0, TypeType = typeof(int))]
		[Key(1)]
		[Generator(2, Class = "native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Org")]
		public virtual Point Org { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Height")]
		public virtual Point Height { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Offset")]
		public virtual Point Offset { get; set; }

		[Property(Column = "buttom_radius")]
		public virtual double ButtomRadius { get; set; }

		[Property(Column = "top_radius")]
		public virtual double TopRadius { get; set; }
	}
}
