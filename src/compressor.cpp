#include <fstream>
#include <numeric>
#include <functional>
#include <bitset>
#include "compressor.hpp"
#include "priority_queue.hpp"

using namespace std::string_literals;


struct freq_node {
    unsigned frequency;
    short id{ -1 };
    char8_t character;

    bool operator<(freq_node const& other) const noexcept {
        return frequency < other.frequency;
    }

    bool operator>(freq_node const& other) const noexcept {
        return frequency > other.frequency;
    }
};

struct tree_node {
    short left{ -1 };
    short right{ -1 };
    char8_t character{};

    char8_t value() const noexcept {
        return character;
    }

    bool is_leaf() const noexcept {
        return left == -1 && right == -1;
    }

    short get_child(char8_t bit) const noexcept {
        return bit ? right : left;
    }
};

struct tree_t {
    short size{};
    tree_node nodes[511]{};

    const tree_node& root() const {
        return nodes[0];
    }

    std::size_t size_in_bytes() const {
        return sizeof(size) + size * sizeof(tree_node);
    }

    static tree_t from_bytes(bytes_view bytes) {
        tree_t tree{};
        tree.size = *(short*)(bytes.data());
        std::copy(bytes.data() + sizeof(tree.size), bytes.data() + tree.size_in_bytes(), (byte*)tree.nodes);
        return tree;
    }

    bytes to_bytes() const {
        return bytes((const byte*)this, size_in_bytes());
    }

    short new_node(char8_t character) {
        nodes[size] = tree_node{ .character = character };
        return size++;
    }

    short new_node(short left, short right) {
        nodes[size] = tree_node{ .left = left, .right = right };
        return size++;
    }

    short new_node(tree_node node) {
        nodes[size] = node;
        return size++;
    }

    tree_node get(short index) const noexcept {
        return nodes[index];
    }

    void reverse() {
        std::reverse(std::begin(nodes), std::begin(nodes) + size);
        std::ranges::for_each(nodes, [&](tree_node& node) {
            if (node.is_leaf()) {
                node.left = -1;
                node.right = -1;
            }
            else {
                node.left = size - node.left - 1;
                node.right = size - node.right - 1;
            }
            });
    }
};

void program(std::span<const char*> argv) {

    std::array args = {
        arg_t {
            .name = "filepath",
            .flag = "-f",
            .description = "File to compress"
        },
        arg_t {
            .name = "output",
            .flag = "-o",
            .description = "File where put file compressed"
        },
        arg_t {
            .name = "uncompress",
            .flag = "-u",
            .description = "Active mode to uncompress",
            .optional = true
        }
    };

    auto [filepath, output, uncompress] = extract_args<3>(argv, args);

    for (auto arg : args) {
        if (!arg.optional && arg.value.empty()) {
            show_help(args);
            break;
        }
    }


    bytes file_content;

    {
        std::ifstream input(filepath, std::ios::binary);
        file_content = bytes{
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()
        };
    }

    bytes file_content_result = uncompress == ""s ? compress(file_content) : ::uncompress(file_content);    

    {
        std::ofstream outfile(output, std::ios::binary);
        outfile.write((const char*)file_content_result.data(), file_content_result.size());
    }
}

void show_help(std::span<arg_t> args) {
    puts("Help: ");
    for (arg_t& arg : args) {
        printf("%16s: %2s \t %40s\n", arg.name.data(), arg.flag.data(), arg.description.data());
    }
}

struct frequency_table {
public:
    struct char_frequency_row {
        byte character;
        unsigned& frequency;
    };

    template <typename frequency_table>
    struct iterator {
        iterator(frequency_table& table, unsigned character)
            : table(table), character(character) {}

        char_frequency_row operator*() noexcept {
            goto_next(true);
            return { byte(character), table.frequencies[character] };
        }

        bool operator!=(iterator const& rhs) const noexcept {
            return &table != &rhs.table || character != rhs.character;
        }

        iterator& operator++() noexcept {
            goto_next();
            return *this;
        }

    private:
        void goto_next(bool count_this = false) noexcept {
            if (count_this && table.frequency_of(character) > 0) return;
            character++;
            while (table.frequency_of(character) == 0 && character < 256) character++;
        }

        frequency_table& table;
        unsigned character;
    };

    constexpr unsigned frequency_of(byte character) const noexcept {
        return frequencies[character];
    }

    auto begin() const {
        return iterator(*this, 0);
    }

    auto begin() {
        return iterator(*this, 0);
    }

    auto end() const {
        return iterator(*this, 256);
    }

    auto end() {
        return iterator(*this, 256);
    }

    unsigned& operator[](byte byte) noexcept {
        return frequencies[byte];
    }

private:
    unsigned frequencies[256]{};
};


frequency_table generate_frequency_map(bytes const& file_content) {
    frequency_table table{};

    for (auto byte : file_content) {
        table[byte]++;
    }

    return table;
}


tree_t generate_char_tree(frequency_table table) {
    tree_t char_tree{};
    priority_queue<freq_node, 256> sorted_frequencies{};

    for (auto [charater, frequency] : table) {
        sorted_frequencies.insert(freq_node{ .frequency = frequency, .character = charater });
    }

    while (sorted_frequencies.size() > 1) {
        auto p = sorted_frequencies.pop();
        auto q = sorted_frequencies.pop();

        auto id = char_tree.new_node(
            p.id == -1 ? char_tree.new_node(p.character) : p.id,
            q.id == -1 ? char_tree.new_node(q.character) : q.id
        );

        sorted_frequencies.insert(freq_node{ .frequency = p.frequency + q.frequency, .id = id });
    }

    char_tree.reverse();
    return char_tree;
}


struct find_characters {
public:
    constexpr find_characters(tree_t const& tree) : tree(tree) {
        _find_characters(tree.nodes[0], u8"");
    }

    char_map encode() const noexcept {
        return map;
    }

private:
    // busqueda recursiva en un arbol binario
    void _find_characters(tree_node root, bytes code) {
        if (root.is_leaf()) {
            map[root.character] = code;
        }
        else {
            _find_characters(tree.get(root.left), code + u8"0");
            _find_characters(tree.get(root.right), code + u8"1");
        }
    }

    char_map map{};
    tree_t const& tree;
};


char_map generate_char_map(tree_t const& tree) {
    // busca todos los caracteres en el arbol y los devuelve
    // en un mapa con su nuevo código
    return find_characters(tree).encode();
}

struct bitstream {
    bitstream(bytes& bytes) : _bytes(bytes) {
        current_bit = _bytes.size() * 8;
    }

    // los bytes vienen en formato "1011101011..."
    // es decir, cada byte representa un solo bit
    bitstream& operator<<(bytes input_bytes) {

        for (byte byte : input_bytes) {
            set(bit_of(byte));
            next_bit();
        }

        return *this;
    }

private:
    void set(bool value) {
        if (current_byte() >= _bytes.size()) {
            _bytes.push_back(0);
        }
        _bytes[current_byte()] |= (value << current_sub_bit());
    }

    void next_bit() {
        current_bit++;
    }

    std::size_t current_byte() {
        return current_bit / 8;
    }

    std::size_t current_sub_bit() {
        return 7 - current_bit % 8;
    }

    bool bit_of(byte byte) {
        return byte - u8'0';
    }

    bytes& _bytes;
    std::size_t current_bit{};
};


struct file_content_struct {

    static file_content_struct from_bytes(bytes const& bytes) {
        file_content_struct file_content;
        auto current_position = bytes.data();

        file_content.size_in_bytes = *reinterpret_cast<std::size_t const*>(current_position);
        current_position += sizeof(file_content.size_in_bytes);

        file_content.char_tree = tree_t::from_bytes(bytes_view(current_position, bytes.data() + bytes.size()));
        current_position += file_content.char_tree.size_in_bytes();

        file_content.compressed_content = ::bytes(::bytes::const_iterator(current_position), bytes.end());

        return file_content;
    }

    bytes to_bytes() const {
        bytes result;
        result.reserve(compressed_content.size() + sizeof(size_in_bytes) + char_tree.size_in_bytes());
        result.append((byte const*)&size_in_bytes, sizeof(size_in_bytes));
        result.append(char_tree.to_bytes());
        result.append(compressed_content);
        return result;
    }

    std::size_t size_in_bytes{};
    tree_t char_tree{};
    bytes compressed_content{};
};


bytes compress(bytes const& file_content) {
    file_content_struct file_compressed_struct{};

    // primero se genera una tabla con las frecuencias
    // de cada caracter en todo el archivo, con estas
    // se crea el árbol de huffman, y a partir de este
    // ultimo se genera una tabla en la cual estará
    // la nueva forma de representar cada caracter
    file_compressed_struct.size_in_bytes = file_content.size();

    file_compressed_struct.char_tree = generate_char_tree(
        generate_frequency_map(file_content)
    );

    auto char_map = generate_char_map(file_compressed_struct.char_tree);

    file_compressed_struct.compressed_content.reserve(file_content.size());

    bitstream file_content_compressed_bitstream{ file_compressed_struct.compressed_content };

    for (byte byte : file_content) {
        file_content_compressed_bitstream << char_map[byte];
    }

    return file_compressed_struct.to_bytes();
}


struct bitreader {
    bitreader(const bytes& bytes)
        : _bytes(bytes) {}

    bool get() const {
        return (_bytes[current_byte()] & (1 << current_sub_bit())) != 0;
    }

    void next() {
        current_bit++;
    }

    std::size_t current_byte() const {
        return current_bit / 8;
    }
    
    std::size_t current_sub_bit() const {
        return 7 - current_bit % 8;
    }

private:
    bytes const& _bytes;
    std::size_t current_bit{};
};


bytes uncompress(bytes const& file_content) {
    bytes uncompressed_content;
    file_content_struct file_compressed_struct = file_content_struct::from_bytes(file_content);

    uncompressed_content.reserve(file_compressed_struct.size_in_bytes);

    bitreader reader(file_compressed_struct.compressed_content);

    tree_node current_node = file_compressed_struct.char_tree.root();
    while (uncompressed_content.size() < file_compressed_struct.size_in_bytes) {
        if (current_node.is_leaf()) {
            uncompressed_content.push_back(current_node.character);
            current_node = file_compressed_struct.char_tree.root();
        }

        auto bit = reader.get();
        
        current_node = bit 
            ? file_compressed_struct.char_tree.get(current_node.right) 
            : file_compressed_struct.char_tree.get(current_node.left);
        
        reader.next();
    }

    return uncompressed_content;
}