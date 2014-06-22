using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;

namespace DbModel
{
	[Class(Table="box")]
	public class Box
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name="Org")]
		public virtual Point Org { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Length")]
		public virtual Point Length { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Width")]
		public virtual Point Width { get; set; }

		[Point(After = typeof(IdAttribute), Name = "Height")]
		public virtual Point Height { get; set; }
	}
}
