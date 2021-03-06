#!/usr/bin/python

# -----------------------------------------------------------------------------
# Copyright (c) 2014-2016 Leandro T. C. Melo (ltcmelo@gmail.com)
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
# USA
# -----------------------------------------------------------------------------

# -------------------------- #
# --- The UaiSo! Project --- #
# -------------------------- #

import operator
import GenLib


_token_map = {}
_sorted_tokens = []
_alias_tokens = set()


def write_bison_token_spec(bison_file):
    """ Write the bison %token declaration """

    print "Writing to %s" % bison_file

    token_decls = ""
    for token in _sorted_tokens:
        # TODO: Filter tokens only relevant to this parser.
        token_num = token[1][0]

        if int(token_num) < 258:
            # A single character ASCII token, skip it and let Bison define it.
            continue

        # Assemble the %token declarations.
        if token[1][1]:
            if token[0] in _alias_tokens:
                # We don't want the aliases in Bison's declarations.
                continue
            token_decl = "%%token %s %s %s\n" % (token[0], token_num, token[1][1])
        else:
            token_decl = "%%token %s %s\n" % (token[0], token_num)
        token_decls += token_decl

    with open(bison_file, "r") as f:
        content = f.read()

    mark = (
        "    /*--------------------------------------------------*/\n"
        "    /*---            Token declarations              ---*/\n"
        "    /*---                                            ---*/\n"
        "    /*---  This section is AUTOMATICALLY GENERATED.  ---*/\n"
        "    /*--- Do NOT edit manually, changes will be lost ---*/\n"
        "    /*---       Please refer to Tokens.def           ---*/\n"
        "    /*--------------------------------------------------*/\n"
    )

    begin = content.find(mark)
    end = content.find(
        "    /*------------------------------------------*/\n"
        "    /*--- Tokens AUTOMATICALLY GENERATED end ---*/\n"
        "    /*------------------------------------------*/\n"
    )

    if begin == -1 or end == -1:
        raise Exception("%s got messed up" % bison_file)

    new_content = content[:begin + len(mark)]
    new_content += token_decls
    new_content += content[end:]

    with open(bison_file, "w") as f:
        f.write(new_content)


def write_token_names():

    token_names_file = "Parsing/TokenName.cpp"

    print "Creating %s" % token_names_file

    content = GenLib.cpp_file_header()
    content += (
        "#include \"Parsing/Token.h\"\n"
        "\n"
        "namespace uaiso {\n"
        "\n"
        "std::unordered_map<std::uint16_t, const char*> tokenName {\n"
    )

    # Put tokens and names in a map, following the naming convention from the enumeration file.
    for token in _sorted_tokens:
        if token[0].startswith("BEGIN_") or token[0].startswith("END_"):
            content += "    { %s, %s },\n" % (token[0], token[1][1])
        else:
            content += "    { TK_%s, %s },\n" % (token[0], token[1][1])

    content += (
        "};\n"
        "\n"
        "} // namespace uaiso\n"
    )

    with open(token_names_file, "w") as f:
        f.write(content)


def write_tokens():

    token_file = "Parsing/Token.h"

    print "Creating %s" % token_file

    content = GenLib.cpp_file_header()
    content += (
        "#ifndef UAISO_TOKEN_H__\n"
        "#define UAISO_TOKEN_H__\n"
        "\n"
        "#include \"Common/Config.h\"\n"
        "#include <cstdint>\n"
        "#include <iostream>\n"
        "#include <unordered_map>\n"
        "\n"
        "namespace uaiso {\n"
        "\n"
        "/* Tokens are unified, it's reponsibility of a lexer to provide only\n"
        "   the relevant tokens for a particular language. */\n"
        "\n"
        "enum Token : std::uint16_t\n"
        "{\n"
    )

    # Declare enum items.
    for token in _sorted_tokens:
        if token[0].startswith("BEGIN_") or token[0].startswith("END_"):
            # It's to indicate a token range, not a token itself.
            enum_item = "    %s = %s,\n" % (token[0], token[1][0])
        else:
            enum_item = "    TK_%s = %s,\n" % (token[0], token[1][0])
        content += enum_item

    content += (
        "};\n"
        "\n"
        "UAISO_API std::ostream& operator<<(std::ostream& os, Token tk);\n"
        "\n"
        "} // namespace uaiso\n"
        "\n"
        "#endif"
    )

    with open(token_file, "w") as f:
        f.write(content)


def build_token_map():
    """ The token specification is expected to have the following format:
        TOKEN KIND SPELLING DATA
        Where DATA is optional and might be the token's number or an alias
        to another token. When DATA is not present, the token's number is
        is taken from incremental counter, starting from the token
        IDENT that begins at 258 (where bison starts numbering non-
        single-character ASCII tokens).
    """

    file_path = "Parsing/Tokens.def"
    token_num_counter = 259 # Bison starts from 258, which matches our IDENT.

    print "Generating tokens from %s" % file_path

    # First thing is to build the token map.
    with open(file_path, "r") as f:
        for line in f:
            line = line.strip() # Get rid of line endings.

            if line.startswith("TOKEN"):
                line = line.split(' ')[1:] # Discard the TOKEN part.

                # The KIND part exists for every token and must not be duplicate.
                token_kind = line[0]
                if token_kind in _token_map:
                    raise Exception("Duplicate token kind %s" % token_kind)

                # The SPELLING must also be specified for every token.
                token_spelling = line[1]

                if len(line) == 3:
                    # This token has explicit DATA, but we need to check whether it's
                    # actually a number, a spelling, or just a reference to another token.
                    # In the later case, the referenced code must have already been seen.
                    token_data = line[2]

                    if not token_data.isdigit():
                        # It's a reference to another token (an alias).
                        if not token_data in _token_map:
                            raise Exception("Could not find matching reference for %s" % token_data)
                        token_data = _token_map[token_data][0]
                        assert(token_data.isdigit())
                        _alias_tokens.add(token_kind)

                    _token_map[token_kind] = [token_data, token_spelling]
                else:
                    # This is a "regular" token, with an incrementing code number.
                    assert(len(line) == 2)
                    _token_map[token_kind] = [str(token_num_counter), token_spelling]
                    token_num_counter += 1

    # Sort the tokens for better reading.
    global _sorted_tokens
    _sorted_tokens = sorted(_token_map.items(), key=operator.itemgetter(0))


def run():
    build_token_map()
    write_tokens()
    write_token_names()
    write_bison_token_spec("D/D.y")
    write_bison_token_spec("Go/Go.y")


if __name__ == "__main__":
    run()
