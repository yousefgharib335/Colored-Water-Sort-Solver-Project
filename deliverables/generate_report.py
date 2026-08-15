from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.pdfbase import pdfmetrics
from reportlab.platypus import (
    Flowable,
    KeepTogether,
    PageBreak,
    Paragraph,
    SimpleDocTemplate,
    Spacer,
    Table,
    TableStyle,
)
from pathlib import Path


OUTPUT = (
    Path(__file__).resolve().parents[1]
    / "output"
    / "pdf"
    / "Colored_Water_Sort_Solver_Report.pdf"
)

NAVY = colors.HexColor("#081018")
PANEL = colors.HexColor("#111D27")
PANEL_2 = colors.HexColor("#EAF1F6")
BLUE = colors.HexColor("#3B78D8")
GREEN = colors.HexColor("#16875B")
RED = colors.HexColor("#D84C68")
INK = colors.HexColor("#17232D")
MUTED = colors.HexColor("#5E7485")
LIGHT = colors.HexColor("#F5F8FA")
WHITE = colors.white


class TubeDiagram(Flowable):
    def __init__(self, width=150 * mm, height=42 * mm):
        super().__init__()
        self.width = width
        self.height = height

    def draw(self):
        c = self.canv
        c.setStrokeColor(colors.HexColor("#91A7B7"))
        c.setLineWidth(1.6)
        tube_w = 19 * mm
        gap = 13 * mm
        total = 4 * tube_w + 3 * gap
        x0 = (self.width - total) / 2
        bottom = 7 * mm
        layer_h = 12 * mm
        fills = [
            [RED, BLUE],
            [BLUE, RED],
            [],
            [],
        ]
        for i in range(4):
            x = x0 + i * (tube_w + gap)
            for layer, fill in enumerate(fills[i]):
                c.setFillColor(fill)
                c.rect(x + 1.5 * mm, bottom + layer * layer_h, tube_w - 3 * mm, layer_h, fill=1, stroke=0)
            c.setStrokeColor(colors.HexColor("#6E8798"))
            c.line(x, bottom + 2 * layer_h, x, bottom)
            c.line(x, bottom, x + tube_w, bottom)
            c.line(x + tube_w, bottom, x + tube_w, bottom + 2 * layer_h)
            c.setFont("Helvetica-Bold", 8)
            c.setFillColor(MUTED)
            c.drawCentredString(x + tube_w / 2, 2 * mm, f"T{i + 1}")


def register_fonts():
    regular = Path(r"C:\Windows\Fonts\segoeui.ttf")
    bold = Path(r"C:\Windows\Fonts\segoeuib.ttf")
    mono = Path(r"C:\Windows\Fonts\consola.ttf")
    if regular.exists() and bold.exists():
        pdfmetrics.registerFont(TTFont("UI", str(regular)))
        pdfmetrics.registerFont(TTFont("UI-Bold", str(bold)))
    if mono.exists():
        pdfmetrics.registerFont(TTFont("Mono", str(mono)))


register_fonts()
FONT = "UI" if "UI" in pdfmetrics.getRegisteredFontNames() else "Helvetica"
BOLD = "UI-Bold" if "UI-Bold" in pdfmetrics.getRegisteredFontNames() else "Helvetica-Bold"
MONO = "Mono" if "Mono" in pdfmetrics.getRegisteredFontNames() else "Courier"


styles = getSampleStyleSheet()
styles.add(ParagraphStyle(name="ReportTitle", fontName=BOLD, fontSize=27, leading=32, textColor=WHITE, alignment=TA_LEFT, spaceAfter=10))
styles.add(ParagraphStyle(name="CoverSub", fontName=FONT, fontSize=12, leading=18, textColor=colors.HexColor("#C6D4DE"), spaceAfter=8))
styles.add(ParagraphStyle(name="Section", fontName=BOLD, fontSize=19, leading=23, textColor=NAVY, spaceBefore=2, spaceAfter=10))
styles.add(ParagraphStyle(name="Subsection", fontName=BOLD, fontSize=12.5, leading=16, textColor=BLUE, spaceBefore=8, spaceAfter=5))
styles.add(ParagraphStyle(name="BodyX", fontName=FONT, fontSize=9.5, leading=14, textColor=INK, spaceAfter=6))
styles.add(ParagraphStyle(name="Small", fontName=FONT, fontSize=8.2, leading=11.5, textColor=MUTED, spaceAfter=4))
styles.add(ParagraphStyle(name="BulletX", fontName=FONT, fontSize=9.2, leading=13, textColor=INK, leftIndent=13, firstLineIndent=-7, bulletIndent=3, spaceAfter=3))
styles.add(ParagraphStyle(name="CodeX", fontName=MONO, fontSize=8.3, leading=12, textColor=colors.HexColor("#E8F0F5"), backColor=PANEL, borderPadding=9, spaceBefore=4, spaceAfter=8))
styles.add(ParagraphStyle(name="Callout", fontName=FONT, fontSize=9.2, leading=13.5, textColor=INK, backColor=colors.HexColor("#EAF4FF"), borderColor=colors.HexColor("#B9D6F6"), borderWidth=0.8, borderPadding=9, spaceBefore=5, spaceAfter=8))
styles.add(ParagraphStyle(name="Pending", fontName=BOLD, fontSize=9.2, leading=13, textColor=colors.HexColor("#9B3B4E"), backColor=colors.HexColor("#FFF0F3"), borderColor=colors.HexColor("#F1B7C3"), borderWidth=0.8, borderPadding=8, spaceBefore=5, spaceAfter=8))


def P(text, style="BodyX"):
    return Paragraph(text, styles[style])


def bullets(items):
    return [P(f"• {item}", "BulletX") for item in items]


def table(data, widths, header=True):
    t = Table([[P(str(cell), "Small") for cell in row] for row in data], colWidths=widths, repeatRows=1 if header else 0, hAlign="LEFT")
    commands = [
        ("VALIGN", (0, 0), (-1, -1), "TOP"),
        ("GRID", (0, 0), (-1, -1), 0.45, colors.HexColor("#C9D6DF")),
        ("LEFTPADDING", (0, 0), (-1, -1), 7),
        ("RIGHTPADDING", (0, 0), (-1, -1), 7),
        ("TOPPADDING", (0, 0), (-1, -1), 6),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 6),
        ("ROWBACKGROUNDS", (0, 1 if header else 0), (-1, -1), [WHITE, LIGHT]),
    ]
    if header:
        commands.extend([
            ("BACKGROUND", (0, 0), (-1, 0), PANEL),
            ("TEXTCOLOR", (0, 0), (-1, 0), WHITE),
        ])
    t.setStyle(TableStyle(commands))
    return t


def header_footer(canvas, doc):
    canvas.saveState()
    if doc.page > 1:
        canvas.setStrokeColor(colors.HexColor("#D5E0E7"))
        canvas.setLineWidth(0.5)
        canvas.line(18 * mm, 282 * mm, 192 * mm, 282 * mm)
        canvas.setFont(FONT, 7.5)
        canvas.setFillColor(MUTED)
        canvas.drawString(18 * mm, 286 * mm, "COLORED WATER SORT SOLVER")
        canvas.drawRightString(192 * mm, 11 * mm, f"Page {doc.page}")
    canvas.restoreState()


def cover(canvas, doc):
    canvas.saveState()
    w, h = A4
    canvas.setFillColor(NAVY)
    canvas.rect(0, 0, w, h, fill=1, stroke=0)
    canvas.setFillColor(BLUE)
    canvas.rect(0, h - 13 * mm, w, 13 * mm, fill=1, stroke=0)
    canvas.setFillColor(GREEN)
    canvas.circle(w - 22 * mm, h - 42 * mm, 10 * mm, fill=1, stroke=0)
    canvas.setFillColor(RED)
    canvas.circle(w - 42 * mm, h - 60 * mm, 6 * mm, fill=1, stroke=0)
    canvas.restoreState()


story = []

# Cover
story += [Spacer(1, 35 * mm), P("DATA STRUCTURES AND PROBLEM SOLVING", "CoverSub"), P("Colored Water Sort<br/>Solver Project", "ReportTitle"), Spacer(1, 7 * mm), P("C++17 breadth-first search solver with a native Windows dashboard", "CoverSub"), Spacer(1, 46 * mm)]
cover_info = [
    ["Course", "Data Structures and Problem Solving"],
    ["Student 1", "[NAME AND ID REQUIRED]"],
    ["Student 2", "[NAME AND ID OR N/A REQUIRED]"],
    ["Repository", "github.com/yousefgharib335/Colored-Water-Sort-Solver-Project"],
    ["Report status", "Draft - student details and presentation link pending"],
    ["Date", "15 August 2026"],
]
ct = Table([[P(a, "CoverSub"), P(b, "CoverSub")] for a, b in cover_info], colWidths=[38 * mm, 115 * mm])
ct.setStyle(TableStyle([("VALIGN", (0, 0), (-1, -1), "TOP"), ("LINEBELOW", (0, 0), (-1, -1), 0.35, colors.HexColor("#314654")), ("TOPPADDING", (0, 0), (-1, -1), 7), ("BOTTOMPADDING", (0, 0), (-1, -1), 7)]))
story += [ct, PageBreak()]

# Executive summary
story += [P("1. Executive Summary", "Section"), P("This project implements the Colored Water Sort puzzle as a C++17 console application. It models each arrangement as a search state and uses breadth-first search (BFS) to explore legal one-layer transfers. Because BFS visits states in increasing path length, the first solution found has the minimum number of moves. A native Win32 dashboard is included as an optional bonus interface for entering puzzles, loading randomized solvable examples, and examining each solution step visually."), P("Requirements coverage", "Subsection")]
coverage = [
    ["Requirement", "Implementation evidence", "Status"],
    ["C++ console solver", "src/main.cpp; command-line input and output", "Complete"],
    ["Minimum-move solution", "BFS with visited-state tracking and parent links", "Complete"],
    ["No-solution reporting", "Frontier exhaustion returns No solution exists.", "Complete"],
    ["Sample inputs", "solvable, no_solution, and already_solved", "Complete"],
    ["GUI bonus", "src/gui_main.cpp native Windows dashboard", "Complete"],
    ["Public repository", "Repository exists; local upload still pending", "Pending"],
    ["Recorded presentation", "Script prepared; recording URL needed", "Pending"],
]
story += [table(coverage, [43 * mm, 91 * mm, 28 * mm]), Spacer(1, 4 * mm), P("Submission note", "Subsection"), P("The LMS submission should contain only this final PDF. Before submission, replace all red pending fields with the confirmed team information and recorded presentation URL.", "Pending"), PageBreak()]

# Responsibilities
story += [P("2. Team Responsibilities", "Section"), P("The specification permits one or two students and requires both students to make meaningful commits and speak in the presentation when a two-person team is used. The following allocation is a practical draft and must be confirmed before submission.")]
responsibilities = [
    ["Member", "Proposed responsibilities", "Evidence to retain"],
    ["Student 1 - [NAME/ID]", "Console solver, BFS state search, input validation, console tests", "Multiple focused commits and test outputs"],
    ["Student 2 - [NAME/ID or N/A]", "Windows GUI, randomized examples, report, presentation/demo preparation", "Multiple focused commits and GUI demonstration"],
    ["Shared", "Review rules, verify minimum moves, record final demo, approve report", "Final checklist and recording"],
]
story += [table(responsibilities, [40 * mm, 82 * mm, 40 * mm]), Spacer(1, 5 * mm), P("If this is a one-student project, change the table to one member and combine all responsibilities. If it is a two-student project, both accounts must contribute meaningful commits rather than one final upload.", "Pending"), P("Development scope", "Subsection")]
story += bullets(["Required console interface: completed in src/main.cpp.", "Bonus graphical interface: completed in src/gui_main.cpp using the native Windows API.", "Testing assets: three specification-focused input files plus compiled local executables.", "Documentation: README build/run guide, this report, and a timed presentation script."]) + [PageBreak()]

# Problem and design
story += [P("3. Problem Definition and Design", "Section"), P("A puzzle contains N tubes of equal capacity C. Values are entered bottom-to-top. Positive integers are colors and 0 marks unused capacity. A legal move removes exactly one top layer from a nonempty source and places it into a destination that is not full and is either empty or has the same top color."), TubeDiagram(), P("Solved-state interpretation", "Subsection"), P("Every nonempty tube must be uniform, and each color must be consolidated into only one tube. Empty tubes are allowed. A uniform tube may be partially full if that color has fewer than C layers in the input."), P("Input validation", "Subsection")]
story += bullets(["N and C must be positive integers.", "Every console tube line contains exactly C nonnegative integers.", "Zeros may appear only after all colored layers in a tube.", "The GUI accepts omitted trailing rows or values as empty spaces and enforces a display limit of 18 tubes and capacity 12.", "The BFS search can become expensive before the GUI's display limits because the number of reachable states grows combinatorially."]) + [P("Move rule used by the program", "Subsection"), P("One transferred layer equals one move. A contiguous block is not transferred as a single move; each layer is counted separately. This matches the program output and minimum-move search."), PageBreak()]

# Data structures
story += [P("4. Data Structures", "Section")]
ds = [
    ["Data structure", "Role", "Why selected"],
    ["vector<int> (Tube)", "Stores colors bottom-to-top; back() is the top layer", "Dynamic size, indexed access, O(1) amortized push/pop at the top"],
    ["vector<Tube> (State)", "Stores the complete arrangement", "Simple deep copy and deterministic traversal of tubes"],
    ["queue<size_t>", "BFS frontier of node indices", "FIFO order guarantees increasing move depth"],
    ["unordered_map<string, size_t>", "Maps encoded states to visited node indices", "Average O(1) duplicate detection and prevents cycles"],
    ["vector<SearchNode>", "Stores state, parent index, incoming move, and parent flag", "Stable numeric parent references and efficient path reconstruction"],
    ["unordered_set<int>", "Tracks colors already consolidated in solved-state validation", "Average O(1) duplicate-color check"],
]
story += [table(ds, [40 * mm, 59 * mm, 63 * mm]), Spacer(1, 6 * mm), P("State encoding", "Subsection"), P("Each state is serialized with tube separators and color separators, for example <font name='Mono'>|1,2,|2,1,||</font>. The separators preserve tube boundaries and make the key unambiguous for the visited map."), P("Parent-link reconstruction", "Subsection"), P("Rather than copying an entire move sequence into every frontier entry, each search node stores only the index of its parent and the move used to reach it. Once BFS finds a solved state, the program follows parent indices back to the initial state and reverses the collected moves."), PageBreak()]

# Algorithm
story += [P("5. Breadth-First Search Algorithm", "Section"), P("The initial state is inserted into the node list, FIFO frontier, and visited map. The solver repeatedly removes the oldest frontier node, checks whether it is solved, and generates every ordered source/destination pair. Legal and previously unseen results are appended to the frontier.")]
pseudo = """BFS(initial):
  enqueue initial; mark initial visited
  while frontier is not empty:
    current = dequeue frontier
    if current is solved:
      reconstruct and return its move path
    for every source tube S:
      for every destination tube D where S != D:
        next = copy(current)
        if one legal layer can move S -> D
           and next has not been visited:
          store next with parent=current and move=(S,D)
          mark visited; enqueue next
  return no solution"""
story += [P(pseudo.replace("\n", "<br/>"), "CodeX"), P("Why the result is minimum", "Subsection"), P("All edges represent one layer transfer and therefore have equal cost. BFS explores all states at distance d before any state at distance d+1. Consequently, when a solved state is first removed from the frontier, no solution with fewer moves can exist."), P("Why the solver terminates", "Subsection"), P("The puzzle has a finite set of arrangements. Each arrangement is inserted into the visited map at most once. BFS therefore either finds a solution or exhausts every reachable state and reports that no solution exists."), P("GUI random examples", "Subsection"), P("The dashboard shuffles complete color sets into initially filled tubes, keeps two empty tubes, rejects already-solved arrangements, and runs the same BFS solver before displaying an example. Thus every generated example shown to the user is unsolved and verified solvable."), PageBreak()]

# Complexity
story += [P("6. Time and Memory Complexity", "Section"), P("Let N be the number of tubes, C the capacity, and V the number of distinct reachable states explored before completion.")]
complexity = [
    ["Operation", "Cost", "Reason"],
    ["Solved-state check", "O(NC)", "At most C layers are inspected in each of N tubes"],
    ["Ordered move pairs per state", "O(N^2)", "Every source is paired with every different destination"],
    ["Copy and encode one candidate", "O(NC)", "The full vector-of-vectors state is copied and serialized"],
    ["Worst-case search time", "O(V N^3 C)", "O(N^2) candidates, each with an O(NC) state copy/encoding"],
    ["Search memory", "O(VNC)", "Nodes and visited keys retain up to V complete states"],
    ["Solution reconstruction", "O(M)", "M parent links are followed for an M-move answer"],
]
story += [table(complexity, [49 * mm, 34 * mm, 79 * mm]), Spacer(1, 6 * mm), P("Practical behavior", "Subsection"), P("V can grow exponentially or combinatorially with puzzle size, so the practical limit is determined by the arrangement, available memory, and search depth rather than only by N and C. The console parser does not impose a fixed maximum, while the GUI intentionally caps display input at 18 tubes and capacity 12. Large unsolved or deeply scrambled puzzles may still require excessive time or memory far below those values."), P("Implementation note", "Callout"), P("The complexity shown here reflects the current implementation, which copies a complete state for every ordered source/destination candidate. A future optimization could test move legality before copying, use a compact numeric encoding, and apply safe symmetry reduction."), PageBreak()]

# File structure
story += [P("7. Code and File Structure", "Section")]
files = [
    ["Path", "Purpose"],
    ["src/main.cpp", "Required console parser, legal-move logic, BFS solver, path reconstruction, and output"],
    ["src/gui_main.cpp", "Bonus Win32 dashboard, shared solver logic, random examples, guide, drawing, and navigation"],
    ["samples/solvable.txt", "Official solvable four-tube example"],
    ["samples/no_solution.txt", "Official valid puzzle with no solution"],
    ["samples/already_solved.txt", "Zero-move solved-state edge case"],
    ["CMakeLists.txt", "C++17 console build configuration with warnings"],
    ["gui/CMakeLists.txt", "Windows GUI build and user32/gdi32 linkage"],
    ["README.md", "Puzzle interpretation, build/run commands, algorithm, and GUI summary"],
]
story += [table(files, [48 * mm, 114 * mm]), Spacer(1, 6 * mm), P("Why this structure was chosen", "Subsection"), P("The required console application remains independent and easy to grade. The optional GUI is isolated in a separate translation unit and build configuration so it does not add platform dependencies to the console target. Sample inputs are plain text and can be redirected directly into the executable for repeatable tests."), P("Build targets", "Subsection"), P("<font name='Mono'>water_sort_solver</font> is portable C++17. <font name='Mono'>water_sort_gui</font> is a Windows subsystem executable that links against user32 and gdi32."), PageBreak()]

# Testing
story += [P("8. Test Results", "Section"), P("The console executable was run on 15 August 2026 using the three sample files in the repository. All observed results matched the expected behavior.")]
tests = [
    ["Case", "Input summary", "Observed result", "Assessment"],
    ["1. Solvable", "N=4, C=2; [1,2], [2,1], empty, empty", "Solution found; minimum 3 moves; final [1,1], empty, [2,2], empty", "Pass"],
    ["2. No solution", "N=2, C=2; [1,2], [2,1]", "No solution exists.", "Pass"],
    ["3. Already solved", "N=3, C=2; [1,1], [2,2], empty", "Solution found; minimum 0 moves; state unchanged", "Pass"],
]
story += [table(tests, [31 * mm, 49 * mm, 60 * mm, 22 * mm]), Spacer(1, 7 * mm), P("Verified move sequence for Case 1", "Subsection"), P("1. Tube 1 -> Tube 3<br/>2. Tube 2 -> Tube 1<br/>3. Tube 2 -> Tube 3", "CodeX"), P("Testing interpretation", "Subsection")]
story += bullets(["Case 1 checks BFS minimum-path behavior and legal transfers into empty and matching-color tubes.", "Case 2 checks complete frontier exhaustion when no destination space exists.", "Case 3 checks the goal test at depth zero and confirms that an already-solved puzzle needs no transfers.", "The final state prints zeros for unused capacity so the output remains in the required fixed-width tube format."]) + [PageBreak()]

# GUI
story += [P("9. Bonus Windows Dashboard", "Section"), P("The GUI is an optional enhancement and does not replace the required console application. It uses only the C++ standard library and native Win32/GDI functions, so no third-party GUI framework is required.")]
gui_features = [
    ["Feature", "Behavior"],
    ["Solve puzzle", "Parses the editor input and displays the minimum move list or no-solution result"],
    ["Random example", "Generates a different unsolved puzzle and validates solvability with BFS"],
    ["Solution explorer", "Draws tubes and lets the user move backward, forward, or directly to the final state"],
    ["Guide and rules", "Explains input order, move rules, solved state, and practical/display limits"],
    ["Responsive layout", "Centers dashboard panels and adjusts drawing dimensions with the window size"],
    ["Input assistance", "Missing GUI rows/values are empty; invalid negative or misplaced-zero inputs are rejected"],
]
story += [table(gui_features, [45 * mm, 117 * mm]), Spacer(1, 7 * mm), P("Limits", "Subsection"), P("The GUI parser accepts at most 18 tubes and capacity 12 for display usability. These are interface limits, not a promise that BFS will solve every puzzle at that size. State-space growth is the governing practical constraint."), P("Color visualization", "Subsection"), P("The dashboard maps integer color codes onto a rotating palette and labels each visible layer when space permits. Codes beyond the palette length reuse colors visually, while the solver still treats their integer values as distinct."), PageBreak()]

# Build and links
story += [P("10. Build, Run, and Submission Links", "Section"), P("Console build and run", "Subsection"), P("cmake -S . -B build<br/>cmake --build build<br/><br/>Get-Content samples/solvable.txt | .\\build\\water_sort_solver.exe", "CodeX"), P("Windows GUI build and run", "Subsection"), P("cmake -S gui -B gui/build<br/>cmake --build gui/build<br/><br/>.\\gui\\build\\water_sort_gui.exe", "CodeX"), P("GitHub repository", "Subsection"), P("<link href='https://github.com/yousefgharib335/Colored-Water-Sort-Solver-Project' color='#3B78D8'>https://github.com/yousefgharib335/Colored-Water-Sort-Solver-Project</link>"), P("Recorded presentation", "Subsection"), P("[RECORDED PRESENTATION URL REQUIRED BEFORE SUBMISSION]", "Pending"), P("Final submission checklist", "Subsection")]
story += bullets(["Confirm student names, IDs, and responsibility allocation on the cover and Section 2.", "Ensure both students have meaningful GitHub commits if this is a two-person team.", "Make the GitHub repository public and verify it opens while signed out.", "Record a presentation of no more than 10 minutes; both students must speak when applicable.", "Insert the presentation URL into this report and verify link permissions.", "Upload only the final PDF to LMS from one team member's account."]) + [Spacer(1, 5 * mm), P("Conclusion", "Subsection"), P("The core project requirements are implemented and validated. BFS guarantees a minimum-length solution under the one-layer-per-move rule, duplicate states are avoided, and parent links reconstruct the exact move sequence. The bonus dashboard provides an accessible visual demonstration without changing the required console interface."), P("End of report", "Small")]


OUTPUT.parent.mkdir(parents=True, exist_ok=True)
doc = SimpleDocTemplate(str(OUTPUT), pagesize=A4, rightMargin=18 * mm, leftMargin=18 * mm, topMargin=19 * mm, bottomMargin=16 * mm, title="Colored Water Sort Solver Project Report", author="Student team - details pending", subject="Data Structures and Problem Solving project report")
doc.build(story, onFirstPage=cover, onLaterPages=header_footer)
print(OUTPUT)
