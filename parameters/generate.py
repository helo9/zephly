#!/bin/env python3
import argparse
from collections import namedtuple
import jinja2

Parameter = namedtuple("Parameter", ["name", "id", "type", "default"])


def parse_file(path):
    valid_types = ["u32", "i32", "f32"]
    int_params = []
    float_params = []

    next_id = 0

    with open(path, "r") as f:
        lines = f.readlines()

    for l in lines:
        l = l.strip()
        if len(l) == 0:
            continue

        if l[0] == "#":
            continue

        parts = l.split(" ")
        name = parts[0]
        typ = parts[1]
        default = parts[2]

        if typ not in valid_types:
            raise TypeError(f"{name} has invalid type {typ}")

        if typ == "u32":
            default = int(default)
        elif typ == "i32":
            default = int(default)
            assert default > 0
        elif typ == "f32":
            default = float(default)

        if typ == "f32":
            float_params.append(Parameter(name, next_id, typ, default))
            next_id += 1
        else:
            int_params.append(Parameter(name, next_id, typ, default))
            next_id += 1

    return (int_params, float_params)


def main():
    parser = argparse.ArgumentParser(description="ffc parameter system generator.")
    parser.add_argument("param_file", metavar="file", help="parameter definition file")
    args = parser.parse_args()

    path = args.param_file
    int_params, float_params = parse_file(path)

    env = jinja2.Environment(
        loader=jinja2.PackageLoader('generate'),
        autoescape=jinja2.select_autoescape()
    )

    header_template = env.get_template("params_template.h")
    code_template = env.get_template("params_template.c")

    nparams = len(int_params) + len(float_params)

    with open("output/params.h", "w") as f:
        f.write(header_template.render(int_params=int_params,
            float_params=float_params,
            nparams=nparams))

    with open("output/params.c", "w") as f:
        f.write(code_template.render(int_params=int_params,
            float_params=float_params,
            nparams=nparams))

    print("Done.")


if __name__ == "__main__":
    main()
