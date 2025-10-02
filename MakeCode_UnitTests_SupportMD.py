#!/usr/bin/env python3
"""
Script to analyze unit tests across different platforms (DX12, UE 5.3, WebGPU)
and show which tests are supported vs unsupported for each platform.
"""

import os
import glob
import ast
from pathlib import Path

def get_all_unit_tests():
    """Get all unit tests by looking for .py files in Techniques/UnitTests/ recursively."""
    unit_tests = []

    # Get the current working directory
    cwd = os.getcwd()
    search_path = os.path.join(cwd, "Techniques", "UnitTests", "**", "*.py")
    
    for file_path in glob.glob(search_path, recursive=True):
        # Get relative path from Techniques/UnitTests/
        rel_path = os.path.relpath(file_path, os.path.join(cwd, "Techniques", "UnitTests"))
        # Remove .py extension and convert to the format used in unsupported lists
        test_name = os.path.splitext(rel_path)[0].replace('/', '\\')

        # Skip RunTests.py as it's not a test itself
        if test_name != "RunTests" and test_name != "TestLogic":
            unit_tests.append(test_name)

    return sorted(unit_tests)

def extract_unsupported_tests(file_path):
    """Extract the unsupportedTests list from a Python file."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # Parse the Python file to extract the unsupportedTests list
        tree = ast.parse(content)

        for node in ast.walk(tree):
            if isinstance(node, ast.Assign):
                for target in node.targets:
                    if isinstance(target, ast.Name) and target.id == 'unsupportedTests':
                        if isinstance(node.value, ast.List):
                            unsupported = []
                            for item in node.value.elts:
                                if isinstance(item, ast.Constant):
                                    unsupported.append(item.value)
                                elif isinstance(item, ast.Str):  # For older Python versions
                                    unsupported.append(item.s)
                            return unsupported
    except Exception as e:
        print(f"Error parsing {file_path}: {e}")
        return []

    return []

def analyze_platform_support():
    """Analyze unit test support across all platforms."""

    # Platform files mapping
    platforms = {
        'DX12': 'MakeCode_UnitTests_DX12.py',
        'UE 5.3': 'MakeCode_UnitTests_UE_5_3.py',
        'WebGPU': 'MakeCode_UnitTests_WebGPU.py',
        'Viewer': None  # Viewer supports all tests
    }

    # Get all unit tests
    print("Discovering all unit tests...")
    all_tests = get_all_unit_tests()
    print(f"Found {len(all_tests)} total unit tests\n")

    # Analyze each platform
    platform_data = {}

    for platform_name, filename in platforms.items():
        print(f"Analyzing {platform_name}...")

        if filename is None:
            # Viewer platform supports all tests
            unsupported = []
            unsupported_set = set()
        else:
            # Get unsupported tests for this platform
            raw_unsupported = extract_unsupported_tests(filename)
            # Filter to only include tests that actually exist on disk
            unsupported = [test for test in raw_unsupported if test in all_tests]
            unsupported_set = set(unsupported)

        # Determine supported tests
        supported = [test for test in all_tests if test not in unsupported_set]

        platform_data[platform_name] = {
            'supported': sorted(supported),
            'unsupported': sorted(unsupported),
            'total_tests': len(all_tests),
            'supported_count': len(supported),
            'unsupported_count': len(unsupported)
        }

        print(f"  Supported: {len(supported)}")
        print(f"  Unsupported: {len(unsupported)}")
        print(f"  Support Rate: {len(supported)/len(all_tests)*100:.1f}%\n")

    return all_tests, platform_data

def generate_markdown_table(all_tests, platform_data):
    """Generate a markdown table showing unit test support across all platforms."""

    markdown_content = []

    # Title and overview
    markdown_content.append("# Platform Support")
    markdown_content.append("")
    markdown_content.append("The Viewer interprets the gigi graph using DX12 to allow you to do rapid iteration, debuging, and profiling.  The viewer can also be scripted with python to automate tedious tasks such as gathering data or images for diagrams, or for running tests.")
    markdown_content.append("")
    markdown_content.append("The following code generation targets are also available:")
    markdown_content.append("* DX12")
    markdown_content.append("* WebGPU")
    markdown_content.append("* Unreal Engine 5.3")
    markdown_content.append("")
    markdown_content.append("Below are the details of what is supported on each platform, based on the unit tests that expected to pass on each platform.")
    markdown_content.append("")

    # Platform summary table
    markdown_content.append("## Platform Support Summary")
    markdown_content.append("")
    markdown_content.append("| Platform | Supported Tests | Unsupported Tests | Support Rate |")
    markdown_content.append("|----------|----------------|-------------------|--------------|")

    # Sort platforms by support rate descending
    sorted_platforms = sorted(platform_data.items(), key=lambda x: x[1]['supported_count']/x[1]['total_tests'], reverse=True)

    for platform_name, data in sorted_platforms:
        support_rate = f"{data['supported_count']/data['total_tests']*100:.1f}%"
        markdown_content.append(f"| {platform_name} | {data['supported_count']}/{data['total_tests']} | {data['unsupported_count']}/{data['total_tests']} | {support_rate} |")

    markdown_content.append("")

    # Detailed test support table
    markdown_content.append("## Detailed Test Support by Platform")
    markdown_content.append("")

    # Table header - use sorted platform order
    platform_names = [name for name, _ in sorted_platforms]
    header = "| Test Name | " + " | ".join(platform_names) + " |"
    separator = "|-----------|" + "|".join(["-" * max(8, len(name)) for name in platform_names]) + "|"

    markdown_content.append(header)
    markdown_content.append(separator)

    # Table rows
    for test in sorted(all_tests):
        row = f"| `{test}` |"
        for platform_name in platform_names:
            if test in platform_data[platform_name]['supported']:
                row += " ✅ |"
            else:
                row += " ❌ |"
        markdown_content.append(row)

    return "\n".join(markdown_content)

def generate_cross_platform_analysis_markdown(all_tests, platform_data):
    """Generate markdown for cross-platform analysis."""

    markdown_content = []

    # Find tests supported by all platforms
    supported_by_all = set(all_tests)
    for platform_data_item in platform_data.values():
        supported_by_all &= set(platform_data_item['supported'])

    # Find tests supported by no code generation platforms (excluding Viewer)
    code_gen_platforms = {k: v for k, v in platform_data.items() if k != 'Viewer'}
    supported_by_no_codegen = set()
    for test in all_tests:
        if all(test in platform_data_item['unsupported'] for platform_data_item in code_gen_platforms.values()):
            supported_by_no_codegen.add(test)

    # Find tests with partial support among code generation platforms
    partial_support = set(all_tests) - supported_by_all - supported_by_no_codegen

    markdown_content.append("## Cross-Platform Analysis")
    markdown_content.append("")

    # Tests supported by all platforms
    markdown_content.append(f"### Tests Supported by ALL Platforms ({len(supported_by_all)})")
    markdown_content.append("")
    for test in sorted(supported_by_all):
        markdown_content.append(f"- ✅ `{test}`")
    markdown_content.append("")

    # Viewer-only tests
    markdown_content.append(f"### Tests Supported by Viewer ONLY ({len(supported_by_no_codegen)})")
    markdown_content.append("*These tests work in the Viewer but don't generate code for any platform*")
    markdown_content.append("")
    for test in sorted(supported_by_no_codegen):
        markdown_content.append(f"- 📺 `{test}`")
    markdown_content.append("")

    # Partial support tests
    markdown_content.append(f"### Tests with Partial Code Generation Support ({len(partial_support)})")
    markdown_content.append("")

    # Create a table for partial support
    markdown_content.append("| Test Name | DX12 | UE 5.3 | WebGPU |")
    markdown_content.append("|-----------|------|--------|--------|")

    for test in sorted(partial_support):
        row = f"| `{test}` |"
        for platform_name in ['DX12', 'UE 5.3', 'WebGPU']:
            if test in platform_data[platform_name]['supported']:
                row += " ✅ |"
            else:
                row += " ❌ |"
        markdown_content.append(row)

    markdown_content.append("")

    return "\n".join(markdown_content)

def main():
    """Main function to run the analysis."""
    print("Unit Test Platform Support Analysis")
    print("=" * 80)

    try:
        # Analyze platform support
        all_tests, platform_data = analyze_platform_support()

        # Generate markdown content
        print("\nGenerating markdown report...")
        markdown_content = generate_markdown_table(all_tests, platform_data)

        # Write to file
        output_file = "readme/platformSupport.md"
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(markdown_content)

        print(f"✅ Markdown report generated: {output_file}")

        # Print summary to console
        print(f"\nSummary:")
        for platform_name, data in platform_data.items():
            support_rate = f"{data['supported_count']/data['total_tests']*100:.1f}%"
            print(f"  {platform_name}: {data['supported_count']}/{data['total_tests']} ({support_rate})")

    except Exception as e:
        print(f"Error during analysis: {e}")
        return 1

    return 0

if __name__ == "__main__":
    exit(main())
