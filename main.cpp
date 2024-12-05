#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <utility>

using namespace nlohmann;
using namespace std;
using namespace sf;

bool panning = false;
Vector2f screen_pos = {0, 0};
Vector2f temp_pos = {0, 0};
float scale = 96;

Vector2f position(Vector2f pos) {
    return scale * pos + screen_pos;
}

class Icon {
private:
    string i;
    string n;
    string d;
    float pos = 0;
    vector<Icon> children;
    RectangleShape box;
    bool has_img = false;
    Texture img;
public:
    Icon() = default;
    Icon(string id, string name, string description, const string& image = "") {
        i = std::move(id);
        n = std::move(name);
        d = std::move(description);
        box.setFillColor(Color(127, 138, 168));
        if (!image.empty()) {
            img.loadFromFile("img/" + image + ".png");
            has_img = true;
        }
    }
    void add_child(const Icon& child) {
        children.emplace_back(child);
    }
    void move_by(float distance) {
        pos += distance;
        for (Icon& child: children) child.move_by(distance);
    }
    void move_to(float position) {
        for (Icon& child: children) child.move_by(position - pos);
        pos = position;
    }
    float to_positions() {
        map<string, float> widths;
        float width = 0;
        for (Icon& child: children) {
            float w = child.to_positions();
            widths[child.i] = w;
            width += w;
        }
        float used_width = 0;
        for (Icon& child: children) {
            child.move_by((widths[child.i] - width) / 2 + used_width);
            used_width += widths[child.i];
        }
        if (width >= 1) {
            return width;
        } else return 1;
    }
    void draw(RenderWindow& screen, float level = 0) {
        float min;
        float max;
        for (Icon child: children) {
            Vector2f center(child.pos + float(1)/3, level + float(5)/6);
            Vertex child_line[] = {
                    Vertex(position(center + Vector2f(0, 0.5)), Color::White),
                    Vertex(position(center), Color::White)};
            screen.draw(child_line, 2, Lines);
            if (center.x < min) min = center.x;
            if (center.x > max) max = center.x;
            child.draw(screen, level + 1);
        }
        if (!children.empty()) {
            Vector2f center(pos + float(1)/3, level + float(5)/6);
            Vertex parent_line[] = {
                    Vertex(position(center - Vector2f(0, 0.5)), Color::White),
                    Vertex(position(center), Color::White)};
            screen.draw(parent_line, 2, Lines);
            if (children.size() > 1) {
                Vertex cross_line[] = {
                        Vertex(position(Vector2f(min, center.y)), Color::White),
                        Vertex(position(Vector2f(max, center.y)), Color::White)};
                screen.draw(cross_line, 2, Lines);
            }
        }
        float side_length = scale * float(2)/3;
        box.setSize({side_length, side_length});
        box.setPosition(position({pos, level}));
        screen.draw(box);
        if (has_img) {
            Sprite s(img);
            float x_scale = scale * float(8)/15 / s.getLocalBounds().getSize().x;
            float y_scale = scale * float(8)/15 / s.getLocalBounds().getSize().y;
            s.setScale({x_scale, y_scale});
            s.setPosition(position(Vector2f(pos + float(1)/15, level + float(1)/15)));
            screen.draw(s);
        }
    }
    static vector<Icon> load_children_of(json data, const string& parent) {
        vector<Icon> children = {};
        for (auto it = data.begin(); it != data.end(); it++) {
            const string& id = it.key();
            if (data[id]["parent"] == parent) {
                Icon icon;
                if (data[id]["image"].is_null()) icon = Icon(id, data[id]["name"], data[id]["description"]);
                else icon = Icon(id, data[id]["name"], data[id]["description"], data[id]["image"]);
                for (const Icon& child: load_children_of(data, id)) {
                    icon.add_child(child);
                }
                children.emplace_back(icon);
            }
        }
        return children;
    }
};

class Icons {
private:
    Icon root;
public:
    explicit Icons(json data) {
        if (data["root"]["image"].is_null()) root = Icon("root", data["root"]["name"], data["root"]["description"]);
        else root = Icon("root", data["root"]["name"], data["root"]["description"], data["root"]["image"]);
        for (const Icon& child: Icon::load_children_of(data, "root")) root.add_child(child);
        root.move_to(0);
    }
    void set_positions() {
        root.to_positions();
        root.move_to(0);
    }
    void draw(RenderWindow& screen) {
        root.draw(screen);
    }
};

void setup() {
    if (!filesystem::exists("charts/")) filesystem::create_directories("charts/");
    if (!filesystem::exists("img/")) filesystem::create_directories("img/");
}

int main() {
    setup();
    cout << "Tree Chart Name: ";
    string file_name;
    cin >> file_name;

    json data = {};
    ifstream reader("charts/" + file_name + ".json");
    if (reader.good()) {
        cout << "Loading Tree Chart " << file_name << ".json...\n";
        data = json::parse(reader);
    } else {
        cout << "Couldn't load " << file_name << ".json, opening empty chart...\n";
        data["root"]["name"] = "Root";
        data["root"]["description"] = "Welcome to Tree Charter";
    }
    Icons icons(data);
    icons.set_positions();

    auto screen = RenderWindow{{1200, 800}, "Tree Charter"};
    screen_pos = Vector2f(screen.getSize() / unsigned(2));
    while (screen.isOpen()) {
        screen.clear();
        for (auto event = Event{}; screen.pollEvent(event);) {
            if (event.type == Event::Closed) {
                screen.close();
            } else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
                panning = true;
                temp_pos = Vector2f(Mouse::getPosition());
            }
            else if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Right) {
                panning = false;
            }
            else if (event.type == Event::MouseMoved and panning) {
                screen_pos += Vector2f(Mouse::getPosition()) - temp_pos;
                temp_pos = Vector2f(Mouse::getPosition());
            }
        }
        icons.draw(screen);
        screen.display();
    }

    return 0;
}
