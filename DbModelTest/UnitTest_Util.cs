using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using DbModel;
using System.IO;

namespace DbModelTest
{
	[TestClass]
	public class UnitTest_Util
	{
		[TestMethod]
		public void TestInit()
		{
			if (File.Exists("test1.db"))
				File.Delete("test1.db");
			Util util = new Util();
			util.init("test1.db", true);
			Assert.IsTrue(File.Exists("test1.db"));
		}
	}
}
