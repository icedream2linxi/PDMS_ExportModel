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

		[SetRawXml(After = typeof(IdAttribute), Id = "shell_id", Name = "Vertexs", Type = typeof(ShellVertex))]
		public virtual ISet<ShellVertex> Vertexs { get; set; }

		[SetRawXml(After = typeof(IdAttribute), Id = "shell_id", Name = "Faces", Type = typeof(ShellFace))]
		public virtual ISet<ShellFace> Faces { get; set; }

		[ManyToOne(Column = "combine_geometry_id")]
		public virtual CombineGeometry CombineGeometry { get; set; }
	}
}
