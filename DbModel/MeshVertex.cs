using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;

namespace DbModel
{
	[Class(Table="mesh_vertex")]
	public class MeshVertex
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }

		[Point(After = typeof(IdAttribute), Name="Pos")]
		public virtual Point Pos { get; set; }

		[ManyToOne(Column = "mesh_id")]
		public virtual Mesh Mesh { get; set; }
	}
}
