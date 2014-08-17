using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;
using Iesi.Collections.Generic;

namespace DbModel
{
	[Class(Table="mesh")]
	public class Mesh
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }

		[Set(0, OrderBy = "id asc")]
		[OneToMany(1, ClassType = typeof(MeshVertex))]
		[Key(1, Column = "mesh_id")]
		public virtual ISet<MeshVertex> Vertexs { get; set; }

		[Property(Column = "rows")]
		public virtual int Rows { get; set; }

		[Property(Column = "columns")]
		public virtual int Colums { get; set; }

		[ManyToOne(Column = "combine_geometry_id")]
		public virtual CombineGeometry CombineGeometry { get; set; }
	}
}
