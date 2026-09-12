"""ROS-free static checks for the Mid-360 integration contract.

Run with: python -m unittest discover -s tests -v
These checks do not replace colcon build or a hardware/rosbag test.
"""

import ast
from pathlib import Path
import unittest
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "fastlio_ICP" / "src"
UPSTREAM = SOURCE / "FASTLIO2_ROS2"
BRINGUP = SOURCE / "mid360_slam_bringup" / "launch"


def simple_yaml(path):
    """Read the scalar root keys needed by this contract, without PyYAML."""
    result = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.split("#", 1)[0].strip()
        if ":" in line:
            key, value = line.split(":", 1)
            result[key.strip()] = value.strip()
    return result


def launched_packages(path):
    tree = ast.parse(path.read_text(encoding="utf-8"))
    packages = []
    for node in ast.walk(tree):
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
            if node.func.id == "Node":
                for keyword in node.keywords:
                    if keyword.arg == "package":
                        packages.append(ast.literal_eval(keyword.value))
    return set(packages)


class IntegrationContractTest(unittest.TestCase):
    def test_bringup_package_manifest(self):
        manifest = ET.parse(SOURCE / "mid360_slam_bringup" / "package.xml").getroot()
        self.assertEqual(manifest.findtext("name"), "mid360_slam_bringup")
        dependencies = {item.text for item in manifest.findall("exec_depend")}
        self.assertTrue({"fastlio2", "pgo", "localizer"}.issubset(dependencies))

    def test_launch_modes_have_one_map_to_lidar_owner(self):
        mapping = BRINGUP / "mapping.launch.py"
        relocalization = BRINGUP / "relocalization.launch.py"
        self.assertEqual(launched_packages(mapping), {"fastlio2", "pgo", "rviz2"})
        self.assertEqual(
            launched_packages(relocalization), {"fastlio2", "localizer", "rviz2"}
        )
        for path in (mapping, relocalization):
            source = path.read_text(encoding="utf-8")
            self.assertIn("mid360.yaml", source)
            self.assertIn("msg_MID360_launch.py", source)
            self.assertNotIn("/home/wang/", source)

    def test_lio_pgo_and_localizer_topic_frame_contract(self):
        lio = simple_yaml(UPSTREAM / "fastlio2" / "config" / "mid360.yaml")
        pgo = simple_yaml(UPSTREAM / "pgo" / "config" / "pgo.yaml")
        localizer = simple_yaml(UPSTREAM / "localizer" / "config" / "localizer.yaml")
        self.assertEqual(lio["lidar_topic"], "/livox/lidar")
        self.assertEqual(lio["imu_topic"], "/livox/imu")
        self.assertEqual(lio["world_frame"], "lidar")
        self.assertEqual(lio["body_frame"], "body")
        for config in (pgo, localizer):
            self.assertEqual(config["cloud_topic"], "/fastlio2/body_cloud")
            self.assertEqual(config["odom_topic"], "/fastlio2/lio_odom")
            self.assertEqual(config["map_frame"], "map")
            self.assertEqual(config["local_frame"], lio["world_frame"])

    def test_no_unimplemented_lio_map_save_switch(self):
        lio_yaml = (UPSTREAM / "fastlio2" / "config" / "mid360.yaml").read_text(
            encoding="utf-8"
        )
        self.assertNotIn("\npcd_save_en:", lio_yaml)
        self.assertNotIn("\nmap_save_path:", lio_yaml)

    def test_existing_pgo_entry_uses_mid360_config(self):
        source = (UPSTREAM / "pgo" / "launch" / "pgo_launch.py").read_text(
            encoding="utf-8"
        )
        self.assertIn('"mid360.yaml"', source)

    def test_pgo_save_does_not_delete_existing_maps(self):
        source = (UPSTREAM / "pgo" / "src" / "pgo_node.cpp").read_text(
            encoding="utf-8"
        )
        self.assertIn("last_message_time = -std::numeric_limits<double>::infinity()", source)
        self.assertNotIn("remove_all(patches_dir)", source)
        self.assertIn("output directory must be empty", source)

    def test_relocalization_ack_is_not_alignment_success(self):
        source = (UPSTREAM / "localizer" / "src" / "localizer_node.cpp").read_text(
            encoding="utf-8"
        )
        self.assertIn("ICP alignment pending", source)
        self.assertIn("if (m_state.localize_success)", source)


if __name__ == "__main__":
    unittest.main()
