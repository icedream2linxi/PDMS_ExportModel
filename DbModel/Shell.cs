using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NHibernate.Mapping.Attributes;
using Iesi.Collections.Generic;

namespace DbModel
{
	[Class(Table="shell")]
	public class Shell
	{
		[Id(0, TypeType=typeof(int))]
		[Key(1)]
		[Generator(2, Class="native")]
		public virtual int ID { get; set; }

		[Set(0, OrderBy = "id asc")]
		[Key(1, Column = "shell_id")]
		[OneToMany(1, ClassType = typeof(ShellVertex))]
		public virtual ISet<ShellVertex> Vertexs { get; set; }

		[Set(0, OrderBy = "id asc")]
		[Key(1, Column = "shell_id")]
		[OneToMany(1, ClassType = typeof(ShellFace))]
		public virtual ISet<ShellFace> Faces { get; set; }

		[ManyToOne(Column = "combine_geometry_id")]
		public virtual CombineGeometry CombineGeometry { get; set; }
	}
}
